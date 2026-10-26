#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"


typedef struct {

    Token *tokens;

    int current;

    int tokenCount;

} Parser;


void initParser(
    Parser *parser,
    Token tokens[],
    int tokenCount
);


Token *peek(Parser *parser);


Token *advance(Parser *parser);


int expect(
    Parser *parser,
    TokenType type
);


ASTNode *parsePrimary(Parser *parser);


ASTNode *parseTerm(Parser *parser);


ASTNode *parseExpression(Parser *parser);

ASTNode *parseComparison(Parser *parser);

#endif