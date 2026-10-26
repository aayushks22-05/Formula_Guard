#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "normalize.h"

/* Convert 0-indexed column integer to letters (e.g. 0 -> "A", 27 -> "AB") */
void colIndexToName(int col, char *buffer, size_t bufSize) {
    if (bufSize == 0 || buffer == NULL) return;

    char temp[16];
    int idx = 0;
    int c = col;

    do {
        temp[idx++] = (char)('A' + (c % 26));
        c = (c / 26) - 1;
    } while (c >= 0);

    size_t outLen = 0;
    for (int i = idx - 1; i >= 0 && outLen < bufSize - 1; i--) {
        buffer[outLen++] = temp[i];
    }
    buffer[outLen] = '\0';
}

/*
 * Parse a cell address string into CellCoord.
 * Handles A1, $A1, A$1, $A$1, AA27, etc.
 */
int parseCellCoord(const char *cellStr, CellCoord *coord) {
    if (cellStr == NULL || coord == NULL || cellStr[0] == '\0') {
        return 0;
    }

    int i = 0;
    coord->isColAbs = 0;
    coord->isRowAbs = 0;
    coord->col = 0;
    coord->row = 0;

    /* Leading $ indicates absolute column */
    if (cellStr[i] == '$') {
        coord->isColAbs = 1;
        i++;
    }

    int colLettersStart = i;
    int colVal = 0;

    while (cellStr[i] != '\0' && isalpha((unsigned char)cellStr[i])) {
        colVal = colVal * 26 + (toupper((unsigned char)cellStr[i]) - 'A' + 1);
        i++;
    }

    if (i == colLettersStart) {
        return 0; /* Missing column letters */
    }

    /* 0-indexed column */
    coord->col = colVal - 1;

    /* Check for $ before row */
    if (cellStr[i] == '$') {
        coord->isRowAbs = 1;
        i++;
    }

    int rowDigitsStart = i;
    int rowVal = 0;

    while (cellStr[i] != '\0' && isdigit((unsigned char)cellStr[i])) {
        rowVal = rowVal * 10 + (cellStr[i] - '0');
        i++;
    }

    if (i == rowDigitsStart || cellStr[i] != '\0' || rowVal <= 0) {
        return 0; /* Missing or invalid row number */
    }

    coord->row = rowVal;
    return 1;
}

/*
 * Normalize a single cell reference relative to baseCell.
 */
int normalizeCellRef(const char *cellStr, const char *baseCellStr, char *outBuffer, size_t bufSize) {
    if (cellStr == NULL || baseCellStr == NULL || outBuffer == NULL || bufSize == 0) {
        return 0;
    }

    CellCoord target;
    CellCoord base;

    if (!parseCellCoord(cellStr, &target)) {
        return 0;
    }
    if (!parseCellCoord(baseCellStr, &base)) {
        return 0;
    }

    /* Case 1: Fully Relative -> REL[dCol, dRow] */
    if (!target.isColAbs && !target.isRowAbs) {
        int dCol = target.col - base.col;
        int dRow = target.row - base.row;
        snprintf(outBuffer, bufSize, "REL[%d,%d]", dCol, dRow);
        return 1;
    }

    /* Case 2: Fully Absolute -> ABS[Col, Row] */
    if (target.isColAbs && target.isRowAbs) {
        char colName[16];
        colIndexToName(target.col, colName, sizeof(colName));
        snprintf(outBuffer, bufSize, "ABS[%s,%d]", colName, target.row);
        return 1;
    }

    /* Case 3: Mixed Reference */
    char colPart[32];
    char rowPart[32];

    if (target.isColAbs) {
        char colName[16];
        colIndexToName(target.col, colName, sizeof(colName));
        snprintf(colPart, sizeof(colPart), "ABS(%s)", colName);
    } else {
        int dCol = target.col - base.col;
        snprintf(colPart, sizeof(colPart), "REL(%d)", dCol);
    }

    if (target.isRowAbs) {
        snprintf(rowPart, sizeof(rowPart), "ABS(%d)", target.row);
    } else {
        int dRow = target.row - base.row;
        snprintf(rowPart, sizeof(rowPart), "REL(%d)", dRow);
    }

    snprintf(outBuffer, bufSize, "MIXED[%s,%s]", colPart, rowPart);
    return 1;
}

/* Helper to map binary operator symbols to standard names */
static const char *opName(const char *op) {
    if (strcmp(op, "+") == 0) return "ADD";
    if (strcmp(op, "-") == 0) return "SUB";
    if (strcmp(op, "*") == 0) return "MUL";
    if (strcmp(op, "/") == 0) return "DIV";
    if (strcmp(op, "=") == 0) return "EQ";
    if (strcmp(op, "<>") == 0) return "NEQ";
    if (strcmp(op, "<") == 0) return "LT";
    if (strcmp(op, ">") == 0) return "GT";
    if (strcmp(op, "<=") == 0) return "LTE";
    if (strcmp(op, ">=") == 0) return "GTE";
    return op;
}

/*
 * Recursive AST normalization
 */
int normalizeAST(const ASTNode *node, const char *baseCellStr, char *outBuffer, size_t bufSize) {
    if (node == NULL || outBuffer == NULL || bufSize == 0) {
        return 0;
    }

    switch (node->type) {
        case AST_NUMBER: {
            snprintf(outBuffer, bufSize, "%s", node->value);
            return 1;
        }

        case AST_CELL: {
            return normalizeCellRef(node->value, baseCellStr, outBuffer, bufSize);
        }

        case AST_RANGE: {
            char startBuf[128];
            char endBuf[128];

            if (!normalizeAST(node->left, baseCellStr, startBuf, sizeof(startBuf))) {
                return 0;
            }
            if (!normalizeAST(node->right, baseCellStr, endBuf, sizeof(endBuf))) {
                return 0;
            }
            snprintf(outBuffer, bufSize, "RANGE(%s, %s)", startBuf, endBuf);
            return 1;
        }

        case AST_BINARY_OP: {
            char leftBuf[512];
            char rightBuf[512];

            if (!normalizeAST(node->left, baseCellStr, leftBuf, sizeof(leftBuf))) {
                return 0;
            }
            if (!normalizeAST(node->right, baseCellStr, rightBuf, sizeof(rightBuf))) {
                return 0;
            }

            const char *canonicalOp = opName(node->value);
            snprintf(outBuffer, bufSize, "%s(%s, %s)", canonicalOp, leftBuf, rightBuf);
            return 1;
        }

        case AST_FUNCTION: {
            char argsBuf[1024];
            argsBuf[0] = '\0';
            size_t used = 0;

            for (int i = 0; i < node->childCount; i++) {
                char childBuf[512];
                if (!normalizeAST(node->children[i], baseCellStr, childBuf, sizeof(childBuf))) {
                    return 0;
                }

                if (i > 0) {
                    if (used + 2 < sizeof(argsBuf)) {
                        strcat(argsBuf, ", ");
                        used += 2;
                    }
                }

                size_t childLen = strlen(childBuf);
                if (used + childLen < sizeof(argsBuf)) {
                    strcat(argsBuf, childBuf);
                    used += childLen;
                }
            }

            snprintf(outBuffer, bufSize, "%s(%s)", node->value, argsBuf);
            return 1;
        }

        default:
            return 0;
    }
}

/*
 * Compare two formulas structurally given their respective host cells.
 */
int areFormulasStructurallyEquivalent(
    const ASTNode *astA, const char *cellA,
    const ASTNode *astB, const char *cellB
) {
    if (astA == NULL || astB == NULL || cellA == NULL || cellB == NULL) {
        return 0;
    }

    char normA[2048];
    char normB[2048];

    if (!normalizeAST(astA, cellA, normA, sizeof(normA))) {
        return 0;
    }
    if (!normalizeAST(astB, cellB, normB, sizeof(normB))) {
        return 0;
    }

    return (strcmp(normA, normB) == 0);
}
