#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/ast.h"
#include "../src/normalize.h"

static int passed = 0;
static int failed = 0;

static ASTNode *parseFormula(const char *formula) {
    Token tokens[100];
    int tokenCount = lex(formula, tokens);
    if (tokenCount < 0) {
        return NULL;
    }

    Parser parser;
    initParser(&parser, tokens, tokenCount);

    if (!expect(&parser, TOKEN_EQUAL)) {
        return NULL;
    }

    return parseComparison(&parser);
}

static void testEquivalence(
    const char *testName,
    const char *formA, const char *cellA,
    const char *formB, const char *cellB,
    int expectedEquivalent
) {
    ASTNode *astA = parseFormula(formA);
    ASTNode *astB = parseFormula(formB);

    if (astA == NULL || astB == NULL) {
        printf("[FAIL] %s: parsing failed (astA=%p, astB=%p)\n", testName, (void*)astA, (void*)astB);
        failed++;
        if (astA) freeAST(astA);
        if (astB) freeAST(astB);
        return;
    }

    char normA[1024];
    char normB[1024];
    normalizeAST(astA, cellA, normA, sizeof(normA));
    normalizeAST(astB, cellB, normB, sizeof(normB));

    int actualEquivalent = areFormulasStructurallyEquivalent(astA, cellA, astB, cellB);

    if (actualEquivalent == expectedEquivalent) {
        printf("[PASS] %s\n", testName);
        printf("       Formula A (%s @ %s): %s\n", formA, cellA, normA);
        printf("       Formula B (%s @ %s): %s\n", formB, cellB, normB);
        passed++;
    } else {
        printf("[FAIL] %s\n", testName);
        printf("       Expected: %s, Actual: %s\n",
               expectedEquivalent ? "EQUIVALENT" : "DIFFERENT",
               actualEquivalent ? "EQUIVALENT" : "DIFFERENT");
        printf("       Norm A: %s\n", normA);
        printf("       Norm B: %s\n", normB);
        failed++;
    }

    freeAST(astA);
    freeAST(astB);
}

int main(void) {
    printf("========================================\n");
    printf("FormulaGuard Normalization Tests (Week 5)\n");
    printf("========================================\n\n");

    /* Initial Test Cases from Week 5 Specification */

    /* Case 1: Same relative pattern shifted across rows */
    testEquivalence(
        "Case 1: =B2+C2 in D2 and =B3+C3 in D3 (Same pattern)",
        "=B2+C2", "D2",
        "=B3+C3", "D3",
        1
    );

    /* Case 2: Different operators */
    testEquivalence(
        "Case 2: =B2+C2 vs =B2-C2 (Different operators)",
        "=B2+C2", "D2",
        "=B2-C2", "D2",
        0
    );

    /* Case 3: Operator precedence / AST shape differences */
    testEquivalence(
        "Case 3: =(B2+C2)*D2 vs =B3+C3*D3 (Precedence differences)",
        "=(B2+C2)*D2", "E2",
        "=B3+C3*D3", "E3",
        0
    );

    /* Case 4: Absolute reference ($B$2) with relative reference */
    testEquivalence(
        "Case 4: =$B$2+C2 in D2 vs =$B$2+C3 in D3 (Absolute reference preserved)",
        "=$B$2+C2", "D2",
        "=$B$2+C3", "D3",
        1
    );

    /* Case 5: Different row offsets in same formula cell */
    testEquivalence(
        "Case 5: =B2+C2 vs =B2+C3 in same cell D2 (Different offsets)",
        "=B2+C2", "D2",
        "=B2+C3", "D2",
        0
    );

    /* Additional Cases: Mixed references & Functions */
    testEquivalence(
        "Case 6: Mixed ref $B2+C$1 across rows D2 and D3",
        "=$B2+C$1", "D2",
        "=$B3+C$1", "D3",
        1
    );

    testEquivalence(
        "Case 7: Function with ranges =SUM(A1:B5) in C5 vs =SUM(A2:B6) in C6",
        "=SUM(A1:B5)", "C5",
        "=SUM(A2:B6)", "C6",
        1
    );

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    return failed > 0 ? 1 : 0;
}
