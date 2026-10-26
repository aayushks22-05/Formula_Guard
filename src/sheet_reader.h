#ifndef SHEET_READER_H
#define SHEET_READER_H

#include <stddef.h>
#include "pattern_analysis.h"

#define MAX_CELL_TEXT 256

/* Represents an individual cell within the ingested spreadsheet */
typedef struct {
    char cellAddr[MAX_CELL_LEN]; /* e.g. "D2", "AA10" */
    int row;                     /* 1-indexed row number */
    int col;                     /* 0-indexed column (0=A, 1=B, ...) */
    char rawText[MAX_CELL_TEXT]; /* Raw string content */
    int isFormula;               /* 1 if starts with '=', 0 otherwise */
} SheetCell;

/* In-memory spreadsheet grid */
typedef struct {
    SheetCell *cells;
    int cellCount;
    int cellCapacity;
    int maxRow;
    int maxCol;
} SpreadsheetGrid;

/* Lifecycle functions */
void initSpreadsheetGrid(SpreadsheetGrid *grid);
void freeSpreadsheetGrid(SpreadsheetGrid *grid);

/*
 * Load a spreadsheet file (.xlsx or .csv) into a SpreadsheetGrid.
 * Returns 1 on success, 0 on failure.
 */
int loadSpreadsheet(const char *filePath, SpreadsheetGrid *grid);

/*
 * Extract all formulas located in a specific column (0-based) into a PatternGroupCollection.
 * Returns the count of formulas extracted.
 */
int extractColumnFormulas(
    const SpreadsheetGrid *grid,
    int targetCol,
    PatternGroupCollection *collection
);

/*
 * Extract all formulas located in a specific row (1-based) into a PatternGroupCollection.
 * Returns the count of formulas extracted.
 */
int extractRowFormulas(
    const SpreadsheetGrid *grid,
    int targetRow,
    PatternGroupCollection *collection
);

#endif /* SHEET_READER_H */
