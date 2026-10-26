#include <stdio.h>
#include <string.h>

#include "semantic.h"


/*
 * Convert semantic types into
 * human-readable names.
 */
const char *semanticTypeName(SemanticType type) {

    switch (type) {

        case SEM_NUMBER:
            return "NUMBER";

        case SEM_CELL:
            return "CELL";

        case SEM_RANGE:
            return "RANGE";

        case SEM_BOOLEAN:
            return "BOOLEAN";

        case SEM_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}


/*
 * Convert semantic error categories
 * into readable names.
 */
const char *semanticErrorName(
    SemanticErrorType error
) {

    switch (error) {

        case SEM_ERROR_NONE:
            return "NONE";

        case SEM_ERROR_INVALID_CELL:
            return "INVALID_CELL";

        case SEM_ERROR_INVALID_RANGE:
            return "INVALID_RANGE";

        case SEM_ERROR_INVALID_OPERATOR:
            return "INVALID_OPERATOR";

        case SEM_ERROR_UNKNOWN_FUNCTION:
            return "UNKNOWN_FUNCTION";

        case SEM_ERROR_INVALID_ARGUMENT_COUNT:
            return "INVALID_ARGUMENT_COUNT";

        case SEM_ERROR_INVALID_ARGUMENT_TYPE:
            return "INVALID_ARGUMENT_TYPE";

        default:
            return "UNKNOWN_ERROR";
    }
}


/*
 * Convert semantic errors into
 * user-friendly messages.
 */
const char *semanticErrorMessage(
    SemanticErrorType error
) {

    switch (error) {

        case SEM_ERROR_NONE:
            return "No semantic error";

        case SEM_ERROR_INVALID_CELL:
            return "Invalid cell reference";

        case SEM_ERROR_INVALID_RANGE:
            return "Invalid cell range";

        case SEM_ERROR_INVALID_OPERATOR:
            return "Invalid operator";

        case SEM_ERROR_UNKNOWN_FUNCTION:
            return "Unknown function";

        case SEM_ERROR_INVALID_ARGUMENT_COUNT:
            return "Invalid function argument count";

        case SEM_ERROR_INVALID_ARGUMENT_TYPE:
            return "Invalid function or operator argument type";

        default:
            return "Unknown semantic error";
    }
}


/*
 * Determine whether a semantic type
 * represents a scalar value.
 *
 * NUMBER and CELL are treated as
 * scalar expressions.
 */
int isScalarType(SemanticType type) {

    return (
        type == SEM_NUMBER ||
        type == SEM_CELL
    );
}


/*
 * Validate an Excel-style cell
 * reference.
 *
 * Examples:
 *
 * A1      -> valid
 * AA27    -> valid
 * A0      -> invalid
 * ABC     -> invalid
 * 123     -> invalid
 */
int isValidCellReference(
    const char *value
) {

    if (value == NULL ||
        value[0] == '\0') {

        return 0;
    }


    int i = 0;

    /* Optional $ before column */
    if (value[i] == '$') {
        i++;
    }

    int colStart = i;

    /*
     * Read column letters.
     */
    while (
        value[i] != '\0' &&
        (
            (value[i] >= 'A' &&
             value[i] <= 'Z') ||

            (value[i] >= 'a' &&
             value[i] <= 'z')
        )
    ) {

        i++;
    }


    /*
     * At least one column letter
     * is required.
     */
    if (i == colStart) {

        return 0;
    }

    /* Optional $ before row */
    if (value[i] == '$') {
        i++;
    }

    /*
     * Remember where the row
     * number starts.
     */
    int rowStart = i;


    /*
     * Read row digits.
     */
    while (
        value[i] != '\0' &&
        value[i] >= '0' &&
        value[i] <= '9'
    ) {

        i++;
    }


    /*
     * At least one row digit
     * is required.
     */
    if (i == rowStart) {

        return 0;
    }


    /*
     * No other characters are allowed.
     */
    if (value[i] != '\0') {

        return 0;
    }


    /*
     * Check that the row is not zero.
     */
    int row = 0;

    for (
        int j = rowStart;
        j < i;
        j++
    ) {

        row =
            row * 10 +
            (value[j] - '0');

        /*
         * We only need to know whether
         * the row is greater than zero.
         */
        if (row > 0) {

            break;
        }
    }


    if (row == 0) {

        return 0;
    }


    return 1;
}


/*
 * Perform semantic analysis
 * on an AST node.
 */
SemanticResult analyzeAST(
    ASTNode *node
) {

    SemanticResult result;


    /*
     * Default result.
     */
    result.valid = 1;
    result.type = SEM_ERROR;
    result.error = SEM_ERROR_NONE;


    /*
     * NULL is not a valid AST node.
     */
    if (node == NULL) {

        result.valid = 0;

        return result;
    }


    switch (node->type) {


        /*
         * NUMBER
         */
        case AST_NUMBER:

            result.type = SEM_NUMBER;

            return result;


        /*
         * CELL
         */
        case AST_CELL:

            if (
                !isValidCellReference(
                    node->value
                )
            ) {

                result.valid = 0;
                result.error =
                    SEM_ERROR_INVALID_CELL;

                return result;
            }

            result.type = SEM_CELL;

            return result;


        /*
         * RANGE
         */
        case AST_RANGE: {

            /*
             * A range must have both
             * endpoints.
             */
            if (
                node->left == NULL ||
                node->right == NULL
            ) {

                result.valid = 0;
                result.error =
                    SEM_ERROR_INVALID_RANGE;

                return result;
            }


            /*
             * Both endpoints must be
             * CELL nodes.
             */
            if (
                node->left->type != AST_CELL ||
                node->right->type != AST_CELL
            ) {

                result.valid = 0;
                result.error =
                    SEM_ERROR_INVALID_RANGE;

                return result;
            }


            /*
             * Analyze both endpoints.
             */
            SemanticResult startResult =
                analyzeAST(node->left);

            SemanticResult endResult =
                analyzeAST(node->right);


            /*
             * Invalid endpoint means
             * invalid range.
             */
            if (
                !startResult.valid ||
                !endResult.valid
            ) {

                result.valid = 0;
                result.error =
                    SEM_ERROR_INVALID_RANGE;

                return result;
            }


            result.type = SEM_RANGE;

            return result;
        }


        /*
         * BINARY OPERATOR
         */
        case AST_BINARY_OP: {

            SemanticResult leftResult =
                analyzeAST(node->left);

            /*
             * Propagate an invalid
             * left operand.
             */
            if (!leftResult.valid) {

                result.valid = 0;
                result.error =
                    leftResult.error;

                return result;
            }


            SemanticResult rightResult =
                analyzeAST(node->right);

            /*
             * Propagate an invalid
             * right operand.
             */
            if (!rightResult.valid) {

                result.valid = 0;
                result.error =
                    rightResult.error;

                return result;
            }


            /*
             * Arithmetic operators.
             */
            if (
                strcmp(node->value, "+") == 0 ||
                strcmp(node->value, "-") == 0 ||
                strcmp(node->value, "*") == 0 ||
                strcmp(node->value, "/") == 0
            ) {

                /*
                 * Arithmetic requires
                 * scalar operands.
                 */
                if (
                    !isScalarType(
                        leftResult.type
                    ) ||
                    !isScalarType(
                        rightResult.type
                    )
                ) {

                    result.valid = 0;
                    result.error =
                        SEM_ERROR_INVALID_ARGUMENT_TYPE;

                    return result;
                }


                /*
                 * Arithmetic produces NUMBER.
                 */
                result.type = SEM_NUMBER;

                return result;
            }


            /*
             * Comparison operators.
             */
            if (
                strcmp(node->value, "=") == 0 ||
                strcmp(node->value, "<") == 0 ||
                strcmp(node->value, ">") == 0 ||
                strcmp(node->value, "<=") == 0 ||
                strcmp(node->value, ">=") == 0 ||
                strcmp(node->value, "<>") == 0
            ) {

                /*
                 * Comparisons require
                 * scalar operands.
                 */
                if (
                    !isScalarType(
                        leftResult.type
                    ) ||
                    !isScalarType(
                        rightResult.type
                    )
                ) {

                    result.valid = 0;
                    result.error =
                        SEM_ERROR_INVALID_ARGUMENT_TYPE;

                    return result;
                }


                /*
                 * Comparisons produce BOOLEAN.
                 */
                result.type = SEM_BOOLEAN;

                return result;
            }


            /*
             * Operator is not supported.
             */
            result.valid = 0;
            result.error =
                SEM_ERROR_INVALID_OPERATOR;

            return result;
        }


        /*
         * FUNCTION
         */
        case AST_FUNCTION: {

            /*
             * Every supported function
             * must have arguments.
             */
            if (
                node->children == NULL ||
                node->childCount <= 0
            ) {

                result.valid = 0;
                result.error =
                    SEM_ERROR_INVALID_ARGUMENT_COUNT;

                return result;
            }


            /*
             * SUM
             */
            if (
                strcmp(node->value, "SUM") == 0
            ) {

                for (
                    int i = 0;
                    i < node->childCount;
                    i++
                ) {

                    SemanticResult argumentResult =
                        analyzeAST(
                            node->children[i]
                        );


                    /*
                     * Preserve the original
                     * child error.
                     */
                    if (!argumentResult.valid) {

                        result.valid = 0;
                        result.error =
                            argumentResult.error;

                        return result;
                    }


                    /*
                     * SUM accepts:
                     *
                     * NUMBER
                     * CELL
                     * RANGE
                     */
                    if (
                        argumentResult.type != SEM_NUMBER &&
                        argumentResult.type != SEM_CELL &&
                        argumentResult.type != SEM_RANGE
                    ) {

                        result.valid = 0;
                        result.error =
                            SEM_ERROR_INVALID_ARGUMENT_TYPE;

                        return result;
                    }
                }


                /*
                 * SUM produces NUMBER.
                 */
                result.type = SEM_NUMBER;

                return result;
            }


            /*
             * COUNT
             */
            if (
                strcmp(node->value, "COUNT") == 0
            ) {
                for (
                    int i = 0;
                    i < node->childCount;
                    i++
                ) {
                    SemanticResult argumentResult =
                        analyzeAST(
                            node->children[i]
                        );

                    if (!argumentResult.valid) {
                        result.valid = 0;
                        result.error = argumentResult.error;
                        return result;
                    }

                    if (
                        argumentResult.type != SEM_NUMBER &&
                        argumentResult.type != SEM_CELL &&
                        argumentResult.type != SEM_RANGE
                    ) {
                        result.valid = 0;
                        result.error = SEM_ERROR_INVALID_ARGUMENT_TYPE;
                        return result;
                    }
                }

                result.type = SEM_NUMBER;
                return result;
            }


            /*
             * MAX / MIN / AVERAGE / COUNTA / PRODUCT
             */
            if (
                strcmp(node->value, "MAX") == 0 ||
                strcmp(node->value, "MIN") == 0 ||
                strcmp(node->value, "AVERAGE") == 0 ||
                strcmp(node->value, "COUNTA") == 0 ||
                strcmp(node->value, "PRODUCT") == 0
            ) {
                for (
                    int i = 0;
                    i < node->childCount;
                    i++
                ) {
                    SemanticResult argumentResult =
                        analyzeAST(
                            node->children[i]
                        );

                    if (!argumentResult.valid) {
                        result.valid = 0;
                        result.error = argumentResult.error;
                        return result;
                    }

                    if (
                        argumentResult.type != SEM_NUMBER &&
                        argumentResult.type != SEM_CELL &&
                        argumentResult.type != SEM_RANGE
                    ) {
                        result.valid = 0;
                        result.error = SEM_ERROR_INVALID_ARGUMENT_TYPE;
                        return result;
                    }
                }

                result.type = SEM_NUMBER;
                return result;
            }


            /*
             * ABS (accepts 1 number or cell)
             */
            if (
                strcmp(node->value, "ABS") == 0
            ) {
                if (node->childCount != 1) {
                    result.valid = 0;
                    result.error = SEM_ERROR_INVALID_ARGUMENT_COUNT;
                    return result;
                }

                SemanticResult argRes = analyzeAST(node->children[0]);
                if (!argRes.valid) {
                    result.valid = 0;
                    result.error = argRes.error;
                    return result;
                }

                if (argRes.type != SEM_NUMBER && argRes.type != SEM_CELL) {
                    result.valid = 0;
                    result.error = SEM_ERROR_INVALID_ARGUMENT_TYPE;
                    return result;
                }

                result.type = SEM_NUMBER;
                return result;
            }


            /*
             * ROUND (accepts number/cell, num_digits)
             */
            if (
                strcmp(node->value, "ROUND") == 0
            ) {
                if (node->childCount != 2) {
                    result.valid = 0;
                    result.error = SEM_ERROR_INVALID_ARGUMENT_COUNT;
                    return result;
                }

                for (int i = 0; i < 2; i++) {
                    SemanticResult argRes = analyzeAST(node->children[i]);
                    if (!argRes.valid) {
                        result.valid = 0;
                        result.error = argRes.error;
                        return result;
                    }
                    if (argRes.type != SEM_NUMBER && argRes.type != SEM_CELL) {
                        result.valid = 0;
                        result.error = SEM_ERROR_INVALID_ARGUMENT_TYPE;
                        return result;
                    }
                }

                result.type = SEM_NUMBER;
                return result;
            }


            /*
             * IF
             */
            if (
                strcmp(node->value, "IF") == 0
            ) {

                /*
                 * IF requires exactly
                 * three arguments.
                 */
                if (
                    node->childCount != 3
                ) {

                    result.valid = 0;
                    result.error =
                        SEM_ERROR_INVALID_ARGUMENT_COUNT;

                    return result;
                }


                /*
                 * Analyze condition.
                 */
                SemanticResult conditionResult =
                    analyzeAST(
                        node->children[0]
                    );


                if (!conditionResult.valid) {

                    result.valid = 0;
                    result.error =
                        conditionResult.error;

                    return result;
                }


                /*
                 * IF condition must
                 * produce BOOLEAN.
                 */
                if (
                    conditionResult.type !=
                    SEM_BOOLEAN
                ) {

                    result.valid = 0;
                    result.error =
                        SEM_ERROR_INVALID_ARGUMENT_TYPE;

                    return result;
                }


                /*
                 * Analyze true branch.
                 */
                SemanticResult trueResult =
                    analyzeAST(
                        node->children[1]
                    );


                if (!trueResult.valid) {

                    result.valid = 0;
                    result.error =
                        trueResult.error;

                    return result;
                }


                /*
                 * Analyze false branch.
                 */
                SemanticResult falseResult =
                    analyzeAST(
                        node->children[2]
                    );


                if (!falseResult.valid) {

                    result.valid = 0;
                    result.error =
                        falseResult.error;

                    return result;
                }


                /*
                 * The two branches must
                 * currently have compatible
                 * semantic types.
                 */
                /*
            * Both branches must produce
            * scalar values.
            */
            if (
                !isScalarType(trueResult.type) ||
                !isScalarType(falseResult.type)
            ) {

                result.valid = 0;
                result.error = SEM_ERROR_INVALID_ARGUMENT_TYPE;

                return result;
            }


            /*
            * If both branches are CELL,
            * preserve the CELL type.
            */
            if (
                trueResult.type == SEM_CELL &&
                falseResult.type == SEM_CELL
            ) {

                result.type = SEM_CELL;

                return result;
            }


            /*
            * NUMBER/CELL combinations
            * produce a scalar NUMBER result.
            */
            result.type = SEM_NUMBER;

            return result;
                            
            }


            /*
             * Function name is not supported.
             */
            result.valid = 0;
            result.error =
                SEM_ERROR_UNKNOWN_FUNCTION;

            return result;
        }


        /*
         * Unknown AST node type.
         */
        default:

            result.valid = 0;
            result.error =
                SEM_ERROR_INVALID_OPERATOR;

            return result;
    }
}