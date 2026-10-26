#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <stddef.h>
#include "ast.h"

/*
 * Parsed representation of a cell address.
 * E.g., for "$B$2":
 *   col = 1 (0-indexed for 'B')
 *   isColAbs = 1
 *   row = 2 (1-indexed)
 *   isRowAbs = 1
 */
typedef struct {
    int col;        /* 0-indexed column: A=0, B=1, ... */
    int isColAbs;   /* 1 if preceded by '$', 0 otherwise */
    int row;        /* 1-indexed row number */
    int isRowAbs;   /* 1 if preceded by '$', 0 otherwise */
} CellCoord;

/*
 * Parse a cell address string (e.g. "B2", "$B$2", "B$2", "$B2") into coordinates.
 * Returns 1 on success, 0 on failure.
 */
int parseCellCoord(const char *cellStr, CellCoord *coord);

/*
 * Convert a column index (0-based) to column letters (e.g., 0 -> "A", 27 -> "AB").
 */
void colIndexToName(int col, char *buffer, size_t bufSize);

/*
 * Normalize a single cell reference relative to baseCell.
 * - Relative column: REL[offset_col, offset_row]
 * - Absolute column and row: ABS[col_name, row_num]
 * - Mixed: MIXED[col_part, row_part]
 *
 * Output written to outBuffer. Returns 1 on success, 0 on error.
 */
int normalizeCellRef(const char *cellStr, const char *baseCellStr, char *outBuffer, size_t bufSize);

/*
 * Normalize an entire AST into its canonical string representation
 * given the formula's host cell position (baseCellStr).
 *
 * Example:
 *   =B2+C2 in D2 -> "ADD(REL[-2,0], REL[-1,0])"
 *
 * Output written to outBuffer. Returns 1 on success, 0 on error.
 */
int normalizeAST(const ASTNode *node, const char *baseCellStr, char *outBuffer, size_t bufSize);

/*
 * Compare two formulas structurally given their respective cell locations.
 * Returns 1 if structurally identical, 0 otherwise.
 */
int areFormulasStructurallyEquivalent(
    const ASTNode *astA, const char *cellA,
    const ASTNode *astB, const char *cellB
);

#endif /* NORMALIZE_H */
