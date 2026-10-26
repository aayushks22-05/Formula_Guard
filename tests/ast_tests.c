#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"

static void testAST(const char *label, const char *formula) {
    printf("========================================\n");
    printf("Test: %s\n", label);
    printf("Formula: %s\n", formula);
    printf("----------------------------------------\n");
    printf("Syntax Tree:\n\n");

    Token tokens[100];
    int tokenCount = lex(formula, tokens);

    if (tokenCount < 0) {
        printf("Lexer failed for formula: %s\n\n", formula);
        return;
    }

    Parser parser;
    initParser(&parser, tokens, tokenCount);

    if (!expect(&parser, TOKEN_EQUAL)) {
        printf("Parser failed: formula must start with '='\n\n");
        return;
    }

    ASTNode *root = parseComparison(&parser);

    if (root == NULL) {
        printf("Parsing failed.\n\n");
        return;
    }

    Token *trailing = peek(&parser);
    if (trailing != NULL && trailing->type != TOKEN_EOF) {
        printf("Parser Warning: unexpected trailing token '%s'\n", trailing->text);
    }

    prettyPrintAST(root);
    printf("\n");

    freeAST(root);
}

int main(void) {
    printf("FormulaGuard AST Syntax Tree Tests\n");
    printf("Compiler-Based Spreadsheet Formula AST Visualizer\n\n");

    /* 1. Basic Literals and Binary Arithmetic */
    testAST("Basic Cell Reference", "=A1");
    testAST("Simple Addition", "=A1+B1");
    testAST("Operator Precedence (Multiply before Add)", "=A1+B1*2");
    testAST("Parentheses Overriding Precedence", "=(A1+B1)*2");

    /* 2. Cell Range Operations */
    testAST("Cell Range", "=A1:B10");
    testAST("Range combined with arithmetic", "=A1:B10+5");

    /* 3. Comparisons */
    testAST("Comparison Less Than or Equal", "=A1<=10");
    testAST("Comparison Equality", "=A1=B1");
    testAST("Complex Expression Comparison", "=A1+5>B1*2");

    /* 4. Functions */
    testAST("Single-Argument Function with Range", "=SUM(A1:B5)");
    testAST("Multi-Argument Function", "=SUM(A1,B1,C1)");
    testAST("Nested Function Call in Condition (IF)", "=IF(A1>10,SUM(B1:C5),D1)");
    testAST("Arithmetic inside Function Arguments", "=SUM(A1:B10,C1*2,D1+E1)");

    printf("========================================\n");
    printf("All AST Syntax Tree outputs generated.\n");

    return 0;
}
