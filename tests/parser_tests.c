#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"

int testValidFormula(const char *formula) {

    Token tokens[100];

    int tokenCount = lex(formula, tokens);

    if (tokenCount < 0) {
        return 0;
    }

    Parser parser;

    initParser(
        &parser,
        tokens,
        tokenCount
    );

    if (!expect(&parser, TOKEN_EQUAL)) {
        return 0;
    }

    ASTNode *root = parseComparison(&parser);

    if (root == NULL) {
        return 0;
    }

    Token *token = peek(&parser);

    if (token == NULL || token->type != TOKEN_EOF) {
        freeAST(root);
        return 0;
    }

    freeAST(root);

    return 1;
}


int testInvalidFormula(const char *formula) {

    Token tokens[100];

    int tokenCount = lex(formula, tokens);

    if (tokenCount < 0) {
        return 1;
    }

    Parser parser;

    initParser(
        &parser,
        tokens,
        tokenCount
    );

    if (!expect(&parser, TOKEN_EQUAL)) {
        return 1;
    }

    ASTNode *root = parseComparison(&parser);

    if (root == NULL) {
        return 1;
    }

    Token *token = peek(&parser);

    if (token != NULL && token->type == TOKEN_EOF) {
        freeAST(root);
        return 0;
    }

    freeAST(root);

    return 1;
}

int main() {

    int passed = 0;
    int total = 0;


    printf("FormulaGuard Parser Tests\n\n");


    printf("VALID FORMULAS\n\n");


    total++;
    if (testValidFormula("=A1+B1")) {
        printf("PASS: =A1+B1\n");
        passed++;
    } else {
        printf("FAIL: =A1+B1\n");
    }


    total++;
    if (testValidFormula("=A1:B10")) {
        printf("PASS: =A1:B10\n");
        passed++;
    } else {
        printf("FAIL: =A1:B10\n");
    }


    total++;
    if (testValidFormula("=SUM(A1,B1,C1)")) {
        printf("PASS: =SUM(A1,B1,C1)\n");
        passed++;
    } else {
        printf("FAIL: =SUM(A1,B1,C1)\n");
    }


    total++;
    if (testValidFormula("=A1<=10")) {
        printf("PASS: =A1<=10\n");
        passed++;
    } else {
        printf("FAIL: =A1<=10\n");
    }


    total++;
    if (testValidFormula("=A1+5>B1*2")) {
        printf("PASS: =A1+5>B1*2\n");
        passed++;
    } else {
        printf("FAIL: =A1+5>B1*2\n");
    }


    total++;
    if (testValidFormula("=IF(A1>10,B1,C1)")) {
        printf("PASS: =IF(A1>10,B1,C1)\n");
        passed++;
    } else {
        printf("FAIL: =IF(A1>10,B1,C1)\n");
    }


    total++;
    if (testValidFormula("=SUM(A1:B10,C1*2)")) {
        printf("PASS: =SUM(A1:B10,C1*2)\n");
        passed++;
    } else {
        printf("FAIL: =SUM(A1:B10,C1*2)\n");
    }


    total++;
    if (testValidFormula("=IF(A1>10,SUM(B1:C5),D1)")) {
        printf("PASS: =IF(A1>10,SUM(B1:C5),D1)\n");
        passed++;
    } else {
        printf("FAIL: =IF(A1>10,SUM(B1:C5),D1)\n");
    }


    printf("\nINVALID FORMULAS\n\n");


    total++;
    if (testInvalidFormula("=A1+")) {
        printf("PASS: =A1+ correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =A1+ was accepted\n");
    }


    total++;
    if (testInvalidFormula("=SUM(A1,)")) {
        printf("PASS: =SUM(A1,) correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =SUM(A1,) was accepted\n");
    }


    total++;
    if (testInvalidFormula("=SUM(,A1)")) {
        printf("PASS: =SUM(,A1) correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =SUM(,A1) was accepted\n");
    }


    total++;
    if (testInvalidFormula("=A1:")) {
        printf("PASS: =A1: correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =A1: was accepted\n");
    }


    total++;
    if (testInvalidFormula("=A1>")) {
        printf("PASS: =A1> correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =A1> was accepted\n");
    }


    total++;
    if (testInvalidFormula("=SUM(A1:B10")) {
        printf("PASS: =SUM(A1:B10 correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: =SUM(A1:B10 was accepted\n");
    }


    printf("\n");
    printf("Tests passed: %d/%d\n", passed, total);


    return passed == total ? 0 : 1;
}