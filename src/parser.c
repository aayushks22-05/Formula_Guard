#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"


void initParser(
    Parser *parser,
    Token tokens[],
    int tokenCount
) {

    parser->tokens = tokens;

    parser->current = 0;

    parser->tokenCount = tokenCount;
}


Token *peek(Parser *parser) {

    if (parser->current >= parser->tokenCount) {
        return NULL;
    }

    return &parser->tokens[parser->current];
}


Token *advance(Parser *parser) {

    if (parser->current >= parser->tokenCount) {
        return NULL;
    }

    return &parser->tokens[parser->current++];
}


int expect(
    Parser *parser,
    TokenType type
) {

    Token *token = peek(parser);

    if (token == NULL) {

        printf("Parser Error: unexpected end of input\n");

        return 0;
    }


    if (token->type != type) {

        printf(
            "Parser Error: expected %s but found %s\n",
            tokenTypeName(type),
            tokenTypeName(token->type)
        );

        return 0;
    }


    advance(parser);

    return 1;
}




ASTNode *parsePrimary(Parser *parser) {

    Token *token = peek(parser);


    if (token == NULL) {
        return NULL;
    }

    if (token->type == TOKEN_FUNCTION) {

        Token *functionToken = advance(parser);

        if (!expect(parser, TOKEN_LPAREN)) {
            return NULL;
        }


        ASTNode **arguments = NULL;

        int argumentCount = 0;


        /* Parse first argument */

        ASTNode *argument = parseComparison(parser);

        if (argument == NULL) {

            printf("Parser Error: expected function argument\n");

            return NULL;
        }


        arguments = malloc(sizeof(ASTNode *));

        if (arguments == NULL) {

            printf("Parser Error: memory allocation failed\n");
            freeAST(argument);

            return NULL;
        }


        arguments[0] = argument;

        argumentCount++;

        while (peek(parser) != NULL && peek(parser)->type == TOKEN_COMMA) {

            /* Consume comma */

            advance(parser);


            /* Parse next argument */

            argument = parseComparison(parser);

            if (argument == NULL) {

                printf("Parser Error: expected function argument\n");

                for (int i = 0; i < argumentCount; i++) {
                    freeAST(arguments[i]);
                }
                free(arguments);

                return NULL;
            }


            /* Make room for one more argument */

            ASTNode **newArguments = realloc(
                arguments,
                sizeof(ASTNode *) * (argumentCount + 1)
            );

            if (newArguments == NULL) {

                printf("Parser Error: memory allocation failed\n");

                freeAST(argument);
                for (int i = 0; i < argumentCount; i++) {
                    freeAST(arguments[i]);
                }
                free(arguments);

                return NULL;
            }


            arguments = newArguments;


            /* Store the new argument */

            arguments[argumentCount] = argument;

            argumentCount++;
        }

        if (!expect(parser, TOKEN_RPAREN)) {

            for (int i = 0; i < argumentCount; i++) {
                freeAST(arguments[i]);
            }
            free(arguments);

            return NULL;
        }


        return createFunctionNode(
            functionToken->text,
            arguments,
            argumentCount
        );
    }

    if (token->type == TOKEN_CELL) {

        /* Consume the first cell */

        advance(parser);

        ASTNode *start = createCellNode(token->text);

        if (start == NULL) {

            return NULL;
        }


        /* Check whether this is a range */

        if (peek(parser) != NULL && peek(parser)->type == TOKEN_COLON) {

            /* Consume ':' */

            advance(parser);


            /* The token after ':' must be a cell */

            Token *endToken = peek(parser);

            if (endToken == NULL || endToken->type != TOKEN_CELL) {

                printf("Parser Error: expected cell after ':'\n");

                freeAST(start);

                return NULL;
            }


            /* Consume the ending cell */

            advance(parser);


            ASTNode *end = createCellNode(endToken->text);

            if (end == NULL) {

                freeAST(start);

                return NULL;
            }


            /* Create the RANGE node */

            return createRangeNode(start, end);
        }


        /* Normal cell */

        return start;
    }

    if (token->type == TOKEN_NUMBER) {

        advance(parser);

        return createNumberNode(token->text);
    }

    if (token->type == TOKEN_LPAREN) {

        advance(parser);


        ASTNode *node = parseExpression(parser);


        if (node == NULL) {

            return NULL;
        }


        if (!expect(parser, TOKEN_RPAREN)) {

            freeAST(node);

            return NULL;
        }


        return node;
    }


    return NULL;
}




ASTNode *parseTerm(Parser *parser) {

    ASTNode *left = parsePrimary(parser);


    if (left == NULL) {
        return NULL;
    }


    while (
        peek(parser) != NULL &&
        (peek(parser)->type == TOKEN_MULTIPLY ||
         peek(parser)->type == TOKEN_DIVIDE)
    ) {

        Token *operator = advance(parser);


        ASTNode *right = parsePrimary(parser);


        if (right == NULL) {

            printf(
                "Parser Error: expected value after %s\n",
                operator->text
            );

            freeAST(left);

            return NULL;
        }


        ASTNode *newNode = createBinaryOpNode(
            operator->text,
            left,
            right
        );


        if (newNode == NULL) {
            freeAST(left);
            freeAST(right);
            return NULL;
        }

        left = newNode;
    }


    return left;
}




ASTNode *parseExpression(Parser *parser) {

    ASTNode *left = parseTerm(parser);


    if (left == NULL) {
        return NULL;
    }


    while (
        peek(parser) != NULL &&
        (peek(parser)->type == TOKEN_PLUS ||
         peek(parser)->type == TOKEN_MINUS)
    ) {

        Token *operator = advance(parser);


        ASTNode *right = parseTerm(parser);


        if (right == NULL) {

            printf(
                "Parser Error: expected value after %s\n",
                operator->text
            );

            freeAST(left);

            return NULL;
        }


        ASTNode *newNode = createBinaryOpNode(
            operator->text,
            left,
            right
        );


        if (newNode == NULL) {
            freeAST(left);
            freeAST(right);
            return NULL;
        }

        left = newNode;
    }


    return left;
}

ASTNode *parseComparison(Parser *parser) {

    ASTNode *left = parseExpression(parser);

    if (left == NULL) {
        return NULL;
    }


    while (
        peek(parser) != NULL &&
        (
            peek(parser)->type == TOKEN_EQUAL ||
            peek(parser)->type == TOKEN_LESS ||
            peek(parser)->type == TOKEN_GREATER ||
            peek(parser)->type == TOKEN_LESS_EQUAL ||
            peek(parser)->type == TOKEN_GREATER_EQUAL ||
            peek(parser)->type == TOKEN_NOT_EQUAL
        )
    ) {

        Token *operator = advance(parser);


        ASTNode *right = parseExpression(parser);

        if (right == NULL) {

            printf(
                "Parser Error: expected value after %s\n",
                operator->text
            );

            freeAST(left);

            return NULL;
        }


        ASTNode *newNode = createBinaryOpNode(
            operator->text,
            left,
            right
        );


        if (newNode == NULL) {
            freeAST(left);
            freeAST(right);
            return NULL;
        }

        left = newNode;
    }


    return left;
}