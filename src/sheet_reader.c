#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sheet_reader.h"
#include "normalize.h"

void initSpreadsheetGrid(SpreadsheetGrid *grid) {
    if (grid == NULL) return;
    grid->cells = NULL;
    grid->cellCount = 0;
    grid->cellCapacity = 0;
    grid->maxRow = 0;
    grid->maxCol = 0;
}

void freeSpreadsheetGrid(SpreadsheetGrid *grid) {
    if (grid == NULL) return;
    if (grid->cells != NULL) {
        free(grid->cells);
        grid->cells = NULL;
    }
    grid->cellCount = 0;
    grid->cellCapacity = 0;
    grid->maxRow = 0;
    grid->maxCol = 0;
}

static int addCellToGrid(
    SpreadsheetGrid *grid,
    int row,
    int col,
    const char *text
) {
    if (grid->cellCount >= grid->cellCapacity) {
        int newCap = grid->cellCapacity == 0 ? 64 : grid->cellCapacity * 2;
        SheetCell *newCells = (SheetCell *)realloc(
            grid->cells,
            sizeof(SheetCell) * newCap
        );
        if (newCells == NULL) {
            return 0;
        }
        grid->cells = newCells;
        grid->cellCapacity = newCap;
    }

    SheetCell *c = &grid->cells[grid->cellCount];
    c->row = row;
    c->col = col;

    /* Build address (e.g. D4) */
    char colLetters[16];
    colIndexToName(col, colLetters, sizeof(colLetters));
    snprintf(c->cellAddr, sizeof(c->cellAddr), "%s%d", colLetters, row);

    /* Trim leading and trailing spaces */
    const char *p = text;
    while (*p && isspace((unsigned char)*p)) p++;

    strncpy(c->rawText, p, sizeof(c->rawText) - 1);
    c->rawText[sizeof(c->rawText) - 1] = '\0';

    int len = (int)strlen(c->rawText);
    while (len > 0 && isspace((unsigned char)c->rawText[len - 1])) {
        c->rawText[len - 1] = '\0';
        len--;
    }

    c->isFormula = (c->rawText[0] == '=') ? 1 : 0;

    grid->cellCount++;
    if (row > grid->maxRow) grid->maxRow = row;
    if (col > grid->maxCol) grid->maxCol = col;

    return 1;
}

static int parseCSVStream(FILE *f, SpreadsheetGrid *grid) {
    int ch;
    int currentRow = 1;
    int currentCol = 0;
    char fieldBuf[MAX_CELL_TEXT];
    int fieldLen = 0;
    int inQuotes = 0;

    while ((ch = fgetc(f)) != EOF) {
        if (inQuotes) {
            if (ch == '"') {
                int nextCh = fgetc(f);
                if (nextCh == '"') {
                    if (fieldLen < MAX_CELL_TEXT - 1) {
                        fieldBuf[fieldLen++] = '"';
                    }
                } else {
                    inQuotes = 0;
                    if (nextCh != EOF) {
                        ungetc(nextCh, f);
                    }
                }
            } else {
                if (fieldLen < MAX_CELL_TEXT - 1) {
                    fieldBuf[fieldLen++] = (char)ch;
                }
            }
        } else {
            if (ch == '"') {
                inQuotes = 1;
            } else if (ch == ',') {
                fieldBuf[fieldLen] = '\0';
                addCellToGrid(grid, currentRow, currentCol, fieldBuf);
                fieldLen = 0;
                currentCol++;
            } else if (ch == '\r') {
                int nextCh = fgetc(f);
                if (nextCh != '\n' && nextCh != EOF) {
                    ungetc(nextCh, f);
                }
                fieldBuf[fieldLen] = '\0';
                addCellToGrid(grid, currentRow, currentCol, fieldBuf);
                fieldLen = 0;
                currentRow++;
                currentCol = 0;
            } else if (ch == '\n') {
                fieldBuf[fieldLen] = '\0';
                addCellToGrid(grid, currentRow, currentCol, fieldBuf);
                fieldLen = 0;
                currentRow++;
                currentCol = 0;
            } else {
                if (fieldLen < MAX_CELL_TEXT - 1) {
                    fieldBuf[fieldLen++] = (char)ch;
                }
            }
        }
    }

    if (fieldLen > 0 || currentCol > 0) {
        fieldBuf[fieldLen] = '\0';
        addCellToGrid(grid, currentRow, currentCol, fieldBuf);
    }

    return 1;
}

static int isXlsxFile(const char *path) {
    if (path == NULL) return 0;
    size_t len = strlen(path);
    if (len >= 5 && (strcmp(path + len - 5, ".xlsx") == 0 || strcmp(path + len - 5, ".XLSX") == 0)) {
        return 1;
    }
    return 0;
}

int loadSpreadsheet(const char *filePath, SpreadsheetGrid *grid) {
    if (filePath == NULL || grid == NULL) return 0;
    initSpreadsheetGrid(grid);

    const char *targetCsv = filePath;
    char tempCsv[512];
    int isXlsx = isXlsxFile(filePath);

    if (isXlsx) {
        snprintf(tempCsv, sizeof(tempCsv), "%s.tmp.csv", filePath);
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "cmd /c \"\"C:\\Users\\Aayush's Idea\\AppData\\Local\\Python\\bin\\python.exe\" convert_xlsx.py \"%s\" \"%s\"\"", filePath, tempCsv);
        int ret = system(cmd);
        if (ret != 0) {
            snprintf(cmd, sizeof(cmd), "cmd /c \"python convert_xlsx.py \"%s\" \"%s\"\"", filePath, tempCsv);
            ret = system(cmd);
        }
        if (ret != 0) {
            return 0;
        }
        targetCsv = tempCsv;
    }

    FILE *f = fopen(targetCsv, "r");
    if (f == NULL) {
        if (isXlsx) remove(tempCsv);
        return 0;
    }

    parseCSVStream(f, grid);
    fclose(f);

    if (isXlsx) {
        remove(tempCsv);
    }

    return 1;
}

int extractColumnFormulas(
    const SpreadsheetGrid *grid,
    int targetCol,
    PatternGroupCollection *collection
) {
    if (grid == NULL || collection == NULL) return 0;

    int extracted = 0;
    for (int i = 0; i < grid->cellCount; i++) {
        const SheetCell *cell = &grid->cells[i];
        if (cell->col == targetCol && cell->isFormula) {
            if (processAndGroupFormula(collection, cell->cellAddr, cell->rawText)) {
                extracted++;
            }
        }
    }
    return extracted;
}

int extractRowFormulas(
    const SpreadsheetGrid *grid,
    int targetRow,
    PatternGroupCollection *collection
) {
    if (grid == NULL || collection == NULL) return 0;

    int extracted = 0;
    for (int i = 0; i < grid->cellCount; i++) {
        const SheetCell *cell = &grid->cells[i];
        if (cell->row == targetRow && cell->isFormula) {
            if (processAndGroupFormula(collection, cell->cellAddr, cell->rawText)) {
                extracted++;
            }
        }
    }
    return extracted;
}
