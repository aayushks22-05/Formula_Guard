#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../src/semantic.h"


int passed = 0;
int failed = 0;


/*
 * Check semantic analysis result.
 *
 * A test passes only when:
 *
 * 1. validity matches
 * 2. semantic type matches
 * 3. semantic error matches
 */
void checkResult(
    const char *testName,
    SemanticResult result,
    int expectedValid,
    SemanticType expectedType,
    SemanticErrorType expectedError
) {

    if (
        result.valid == expectedValid &&
        result.type == expectedType &&
        result.error == expectedError
    ) {

        printf("[PASS] %s\n", testName);

        passed++;
    }
    else {

        printf(
            "[FAIL] %s\n"
            "       Expected: valid=%d, type=%s, error=%s\n"
            "       Actual:   valid=%d, type=%s, error=%s\n",

            testName,

            expectedValid,
            semanticTypeName(expectedType),
            semanticErrorName(expectedError),

            result.valid,
            semanticTypeName(result.type),
            semanticErrorName(result.error)
        );

        failed++;
    }
}


/*
 * NUMBER
 */
void testNumber() {

    ASTNode *node =
        createNumberNode("10");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Number produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL
 */
void testCell() {

    ASTNode *node =
        createCellNode("A1");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Cell produces CELL",
        result,
        1,
        SEM_CELL,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * RANGE
 */
void testRange() {

    ASTNode *start =
        createCellNode("A1");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Range produces RANGE",
        result,
        1,
        SEM_RANGE,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL + NUMBER
 */
void testAddition() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createNumberNode("10");

    ASTNode *node =
        createBinaryOpNode(
            "+",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL + NUMBER produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL + CELL
 */
void testCellAddition() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createCellNode("B1");

    ASTNode *node =
        createBinaryOpNode(
            "+",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL + CELL produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL * CELL
 */
void testMultiplication() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createCellNode("B1");

    ASTNode *node =
        createBinaryOpNode(
            "*",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL * CELL produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL > NUMBER
 */
void testGreaterThan() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createNumberNode("10");

    ASTNode *node =
        createBinaryOpNode(
            ">",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL > NUMBER produces BOOLEAN",
        result,
        1,
        SEM_BOOLEAN,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL < CELL
 */
void testLessThan() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createCellNode("B1");

    ASTNode *node =
        createBinaryOpNode(
            "<",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL < CELL produces BOOLEAN",
        result,
        1,
        SEM_BOOLEAN,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL <= CELL
 */
void testLessEqual() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createCellNode("B1");

    ASTNode *node =
        createBinaryOpNode(
            "<=",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL <= CELL produces BOOLEAN",
        result,
        1,
        SEM_BOOLEAN,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL >= NUMBER
 */
void testGreaterEqual() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createNumberNode("10");

    ASTNode *node =
        createBinaryOpNode(
            ">=",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL >= NUMBER produces BOOLEAN",
        result,
        1,
        SEM_BOOLEAN,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * CELL <> CELL
 */
void testNotEqual() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createCellNode("B1");

    ASTNode *node =
        createBinaryOpNode(
            "<>",
            left,
            right
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "CELL <> CELL produces BOOLEAN",
        result,
        1,
        SEM_BOOLEAN,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * RANGE + NUMBER
 */
void testInvalidRangeArithmetic() {

    ASTNode *start =
        createCellNode("A1");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *range =
        createRangeNode(
            start,
            end
        );

    ASTNode *number =
        createNumberNode("10");

    ASTNode *node =
        createBinaryOpNode(
            "+",
            range,
            number
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "RANGE + NUMBER is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
}


/*
 * Valid A1 reference
 */
void testValidCellReference() {

    ASTNode *node =
        createCellNode("A1");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "A1 is a valid cell reference",
        result,
        1,
        SEM_CELL,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * Valid multi-letter reference
 */
void testValidMultiLetterCellReference() {

    ASTNode *node =
        createCellNode("AA27");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "AA27 is a valid cell reference",
        result,
        1,
        SEM_CELL,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * Invalid zero-row reference
 */
void testInvalidZeroRowReference() {

    ASTNode *node =
        createCellNode("A0");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "A0 is an invalid cell reference",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_CELL
    );

    freeAST(node);
}


/*
 * Valid range
 */
void testValidRange() {

    ASTNode *start =
        createCellNode("A1");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "A1:B5 is a valid range",
        result,
        1,
        SEM_RANGE,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * Valid multi-letter range
 */
void testValidMultiLetterRange() {

    ASTNode *start =
        createCellNode("AA1");

    ASTNode *end =
        createCellNode("BC27");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "AA1:BC27 is a valid range",
        result,
        1,
        SEM_RANGE,
        SEM_ERROR_NONE
    );

    freeAST(node);
}


/*
 * Invalid start cell in range
 */
void testInvalidStartCellInRange() {

    ASTNode *start =
        createCellNode("A0");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "A0:B5 is an invalid range",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_RANGE
    );

    freeAST(node);
}


/*
 * Invalid end cell in range
 */
void testInvalidEndCellInRange() {

    ASTNode *start =
        createCellNode("A1");

    ASTNode *end =
        createCellNode("B0");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "A1:B0 is an invalid range",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_RANGE
    );

    freeAST(node);
}


/*
 * SUM(NUMBER)
 */
void testSumWithNumber() {

    ASTNode *argument =
        createNumberNode("10");

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] = argument;

    ASTNode *node =
        createFunctionNode(
            "SUM",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "SUM(NUMBER) produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * SUM(CELL)
 */
void testSumWithCell() {

    ASTNode *argument =
        createCellNode("A1");

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] = argument;

    ASTNode *node =
        createFunctionNode(
            "SUM",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "SUM(CELL) produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * SUM(RANGE)
 */
void testSumWithRange() {

    ASTNode *start =
        createCellNode("A1");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *range =
        createRangeNode(
            start,
            end
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] = range;

    ASTNode *node =
        createFunctionNode(
            "SUM",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "SUM(RANGE) produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * SUM(CELL, NUMBER)
 */
void testSumMultipleArguments() {

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 2);

    arguments[0] =
        createCellNode("A1");

    arguments[1] =
        createNumberNode("10");

    ASTNode *node =
        createFunctionNode(
            "SUM",
            arguments,
            2
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "SUM(CELL, NUMBER) produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * SUM(BOOLEAN)
 */
void testSumWithBoolean() {

    ASTNode *left =
        createCellNode("A1");

    ASTNode *right =
        createNumberNode("10");

    ASTNode *comparison =
        createBinaryOpNode(
            ">",
            left,
            right
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] = comparison;

    ASTNode *node =
        createFunctionNode(
            "SUM",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "SUM(BOOLEAN) is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
    free(arguments);
}


/*
 * Unknown function
 */
void testUnknownFunction() {

    ASTNode *argument =
        createCellNode("A1");

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] = argument;

    ASTNode *node =
        createFunctionNode(
            "XYZ",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Unknown function is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_UNKNOWN_FUNCTION
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF(BOOLEAN, NUMBER, NUMBER)
 */
void testIfWithNumberBranches() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] = condition;
    arguments[1] = createNumberNode("100");
    arguments[2] = createNumberNode("200");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF(BOOLEAN, NUMBER, NUMBER) produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF(BOOLEAN, CELL, CELL)
 */
void testIfWithCellBranches() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] = condition;
    arguments[1] = createCellNode("B1");
    arguments[2] = createCellNode("C1");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF(BOOLEAN, CELL, CELL) produces CELL",
        result,
        1,
        SEM_CELL,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF(CELL, NUMBER, NUMBER)
 */
void testIfWithInvalidCondition() {

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] =
        createCellNode("A1");

    arguments[1] =
        createNumberNode("100");

    arguments[2] =
        createNumberNode("200");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF with CELL condition is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF(BOOLEAN, NUMBER, CELL)
 */
void testIfWithScalarBranches() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] = condition;
    arguments[1] = createNumberNode("100");
    arguments[2] = createCellNode("B1");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF with NUMBER and CELL branches produces NUMBER",
        result,
        1,
        SEM_NUMBER,
        SEM_ERROR_NONE
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF with too few arguments
 */
void testIfWithTooFewArguments() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 2);

    arguments[0] = condition;
    arguments[1] = createNumberNode("100");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            2
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF with too few arguments is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_COUNT
    );

    freeAST(node);
    free(arguments);
}


/*
 * IF with too many arguments
 */
void testIfWithTooManyArguments() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 4);

    arguments[0] = condition;
    arguments[1] = createNumberNode("100");
    arguments[2] = createNumberNode("200");
    arguments[3] = createNumberNode("300");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            4
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF with too many arguments is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_COUNT
    );

    freeAST(node);
    free(arguments);
}


/*
 * Explicit invalid-cell error test
 */
void testInvalidCellError() {

    ASTNode *node =
        createCellNode("A0");

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Invalid cell reports INVALID_CELL",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_CELL
    );

    freeAST(node);
}


/*
 * Explicit invalid-range error test
 */
void testInvalidRangeError() {

    ASTNode *start =
        createCellNode("A0");

    ASTNode *end =
        createCellNode("B5");

    ASTNode *node =
        createRangeNode(
            start,
            end
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Invalid range reports INVALID_RANGE",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_RANGE
    );

    freeAST(node);
}


/*
 * Explicit invalid arithmetic type test
 */
void testInvalidArithmeticTypeError() {

    ASTNode *range =
        createRangeNode(
            createCellNode("A1"),
            createCellNode("B5")
        );

    ASTNode *number =
        createNumberNode("10");

    ASTNode *node =
        createBinaryOpNode(
            "+",
            range,
            number
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Invalid arithmetic reports INVALID_ARGUMENT_TYPE",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
}


/*
 * Explicit unknown-function error test
 */
void testUnknownFunctionError() {

    ASTNode **arguments =
        malloc(sizeof(ASTNode *));

    arguments[0] =
        createCellNode("A1");

    ASTNode *node =
        createFunctionNode(
            "XYZ",
            arguments,
            1
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "Unknown function reports UNKNOWN_FUNCTION",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_UNKNOWN_FUNCTION
    );

    freeAST(node);
    free(arguments);
}


/*
 * Explicit IF argument-count error test
 */
void testIfArgumentCountError() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 2);

    arguments[0] = condition;
    arguments[1] = createNumberNode("100");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            2
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF argument count reports INVALID_ARGUMENT_COUNT",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_COUNT
    );

    freeAST(node);
    free(arguments);
}


/*
 * Explicit IF argument-type error test
 */
void testIfArgumentTypeError() {

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] =
        createCellNode("A1");

    arguments[1] =
        createNumberNode("100");

    arguments[2] =
        createNumberNode("200");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF condition type reports INVALID_ARGUMENT_TYPE",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
    free(arguments);
}


/*
 * Test human-readable semantic
 * error messages.
 */
void testErrorMessages() {

    int valid = 1;


    valid =
        valid &&
        strcmp(
            semanticErrorMessage(
                SEM_ERROR_INVALID_CELL
            ),
            "Invalid cell reference"
        ) == 0;


    valid =
        valid &&
        strcmp(
            semanticErrorMessage(
                SEM_ERROR_INVALID_RANGE
            ),
            "Invalid cell range"
        ) == 0;


    valid =
        valid &&
        strcmp(
            semanticErrorMessage(
                SEM_ERROR_UNKNOWN_FUNCTION
            ),
            "Unknown function"
        ) == 0;


    valid =
        valid &&
        strcmp(
            semanticErrorMessage(
                SEM_ERROR_INVALID_ARGUMENT_COUNT
            ),
            "Invalid function argument count"
        ) == 0;


    valid =
        valid &&
        strcmp(
            semanticErrorMessage(
                SEM_ERROR_INVALID_ARGUMENT_TYPE
            ),
            "Invalid function or operator argument type"
        ) == 0;


    if (valid) {

        printf(
            "[PASS] Semantic error messages\n"
        );

        passed++;
    }
    else {

        printf(
            "[FAIL] Semantic error messages\n"
        );

        failed++;
    }
}
void testIfWithRangeBranch() {

    ASTNode *condition =
        createBinaryOpNode(
            ">",
            createCellNode("A1"),
            createNumberNode("10")
        );

    ASTNode *range =
        createRangeNode(
            createCellNode("B1"),
            createCellNode("C5")
        );

    ASTNode **arguments =
        malloc(sizeof(ASTNode *) * 3);

    arguments[0] = condition;
    arguments[1] = range;
    arguments[2] = createNumberNode("100");

    ASTNode *node =
        createFunctionNode(
            "IF",
            arguments,
            3
        );

    SemanticResult result =
        analyzeAST(node);

    checkResult(
        "IF with RANGE branch is invalid",
        result,
        0,
        SEM_ERROR,
        SEM_ERROR_INVALID_ARGUMENT_TYPE
    );

    freeAST(node);
    free(arguments);
}

/*
 * Main test runner
 */
int main() {

    printf(
        "FormulaGuard Semantic Tests\n\n"
    );


    /*
     * Basic semantic types
     */
    testNumber();

    testCell();

    testRange();


    /*
     * Cell references
     */
    testValidCellReference();

    testValidMultiLetterCellReference();

    testInvalidZeroRowReference();


    /*
     * Arithmetic
     */
    testAddition();

    testCellAddition();

    testMultiplication();

    testInvalidRangeArithmetic();


    /*
     * Comparisons
     */
    testGreaterThan();

    testLessThan();

    testLessEqual();

    testGreaterEqual();

    testNotEqual();


    /*
     * Ranges
     */
    testValidRange();

    testValidMultiLetterRange();

    testInvalidStartCellInRange();

    testInvalidEndCellInRange();


    /*
     * SUM
     */
    testSumWithNumber();

    testSumWithCell();

    testSumWithRange();

    testSumMultipleArguments();

    testSumWithBoolean();

    testUnknownFunction();


    /*
     * IF
     */
    testIfWithNumberBranches();

    testIfWithCellBranches();

    testIfWithInvalidCondition();

    testIfWithScalarBranches();

    testIfWithTooFewArguments();

    testIfWithTooManyArguments();


    /*
     * Error classification
     */
    testInvalidCellError();

    testInvalidRangeError();

    testInvalidArithmeticTypeError();

    testUnknownFunctionError();

    testIfArgumentCountError();

    testIfArgumentTypeError();

    testErrorMessages();


    /*
     * Final results
     */
    printf(
        "\nResults: %d passed, %d failed\n",
        passed,
        failed
    );


    if (failed == 0) {

        printf(
            "All semantic tests passed.\n"
        );

        return 0;
    }


    printf(
        "Some semantic tests failed.\n"
    );

    return 1;
}