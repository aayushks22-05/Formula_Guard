#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "normalize.h"
#include "pattern_analysis.h"
#include "anomaly_detection.h"
#include "sheet_reader.h"

static void auditSpreadsheet(const char *inputPath) {
    printf("==================================================\n");
    printf("FormulaGuard Spreadsheet Audit\n");
    printf("Target File: %s\n", inputPath);
    printf("==================================================\n\n");

    SpreadsheetGrid grid;
    if (!loadSpreadsheet(inputPath, &grid)) {
        printf("Error: Could not open or parse spreadsheet: %s\n", inputPath);
        return;
    }

    printf("Spreadsheet loaded: %d cells (%d rows, %d columns)\n\n",
           grid.cellCount, grid.maxRow, grid.maxCol + 1);

    int totalFormulasInSheet = 0;
    int verticalFormulas = 0;
    int horizontalFormulas = 0;

    /* Count formulas */
    for (int i = 0; i < grid.cellCount; i++) {
        if (grid.cells[i].isFormula) totalFormulasInSheet++;
    }

    printf("======================================================================\n");
    printf("                     FORMULAGUARD AUDIT REPORT                        \n");
    printf("======================================================================\n");
    printf("Target File:       %s\n", inputPath);
    printf("Grid Dimension:    %d rows x %d columns (%d cells)\n", grid.maxRow, grid.maxCol + 1, grid.cellCount);
    printf("Total Formulas:    %d\n", totalFormulasInSheet);
    printf("======================================================================\n\n");

    int totalAnomalies = 0;
    int skippedCols = 0;

    /* 1. Vertical (Column-Wise) Analysis */
    printf("----------------------------------------------------------------------\n");
    printf("1. COLUMN-WISE ANALYSIS\n");
    printf("----------------------------------------------------------------------\n");

    for (int col = 0; col <= grid.maxCol; col++) {
        PatternGroupCollection groups;
        initGroupCollection(&groups);

        int formulaCount = extractColumnFormulas(&grid, col, &groups);
        if (formulaCount > 0) {
            verticalFormulas += formulaCount;

            /* Filter out isolated single formulas if total sheet has multiple formulas */
            if (formulaCount < 2 && grid.maxRow > 2) {
                skippedCols++;
                freeGroupCollection(&groups);
                continue;
            }

            char colName[16];
            colIndexToName(col, colName, sizeof(colName));

            char regionLabel[64];
            snprintf(regionLabel, sizeof(regionLabel), "Column %s", colName);

            AnomalyConfig cfg = defaultAnomalyConfig();
            if (formulaCount >= 3) {
                cfg.minRegionSize = 3;
            }

            AnomalyReport report;
            detectAnomalies(&groups, &cfg, &report);
            printStructuredRegionReport(regionLabel, &groups, &report);

            totalAnomalies += report.anomalyCount;
            freeAnomalyReport(&report);
        }

        freeGroupCollection(&groups);
    }

    if (skippedCols > 0) {
        printf("[i] Notice: %d column(s) with single formulas skipped vertically (evaluated in horizontal rows).\n\n", skippedCols);
    }

    /* 2. Horizontal (Row-Wise) Analysis */
    printf("----------------------------------------------------------------------\n");
    printf("2. ROW-WISE ANALYSIS\n");
    printf("----------------------------------------------------------------------\n");

    int auditedRows = 0;
    int skippedRows = 0;
    for (int row = 1; row <= grid.maxRow; row++) {
        PatternGroupCollection groups;
        initGroupCollection(&groups);

        int formulaCount = extractRowFormulas(&grid, row, &groups);
        /* In sheets with predominantly vertical columns, skip 2-formula horizontal cross-sections */
        if (formulaCount == 2 && verticalFormulas >= 4) {
            skippedRows++;
            freeGroupCollection(&groups);
            continue;
        }

        if (formulaCount >= 2) {
            auditedRows++;
            horizontalFormulas += formulaCount;

            char regionLabel[64];
            snprintf(regionLabel, sizeof(regionLabel), "Row %d", row);

            AnomalyConfig cfg = defaultAnomalyConfig();
            if (formulaCount >= 3) {
                cfg.minRegionSize = 3;
            }

            AnomalyReport report;
            detectAnomalies(&groups, &cfg, &report);
            printStructuredRegionReport(regionLabel, &groups, &report);

            totalAnomalies += report.anomalyCount;
            freeAnomalyReport(&report);
        }

        freeGroupCollection(&groups);
    }

    if (skippedRows > 0) {
        printf("[i] Notice: %d row(s) with isolated 2-formula cross-sections skipped (evaluated vertically in columns).\n\n", skippedRows);
    } else if (auditedRows == 0) {
        printf("No horizontal formula sequences detected across rows.\n\n");
    }

    /* 3. Executive Summary / Health Score */
    double healthScore = 100.0;
    if (totalFormulasInSheet > 0) {
        int consistent = totalFormulasInSheet - totalAnomalies;
        if (consistent < 0) consistent = 0;
        healthScore = ((double)consistent / totalFormulasInSheet) * 100.0;
    }

    printf("======================================================================\n");
    printf("                       EXECUTIVE AUDIT SUMMARY                        \n");
    printf("======================================================================\n");
    printf("Total Scanned Cells:    %d\n", grid.cellCount);
    printf("Total Formulas:         %d\n", totalFormulasInSheet);
    printf("Formula Integrity:      %.1f%% Consistent\n", healthScore);
    printf("Flagged Anomalies:      %d\n", totalAnomalies);
    if (totalAnomalies > 0) {
        printf("Recommendation:         Manual review recommended for flagged outlier cells.\n");
    } else {
        printf("Verdict:                All formula sequences conform to established patterns.\n");
    }
    printf("======================================================================\n");

    freeSpreadsheetGrid(&grid);
}

static void runSingleFormulaDemo(void) {
    printf("FormulaGuard Compiler & Normalization Pipeline\n\n");

    const char *formula = "=SUM(A1:B5)";
    const char *cell = "C5";

    printf("Formula: %s (in cell %s)\n\n", formula, cell);

    Token tokens[100];
    int tokenCount = lex(formula, tokens);
    if (tokenCount == -1) return;

    printf("Tokens:\n\n");
    for (int i = 0; i < tokenCount; i++) {
        printf("%-15s %s\n", tokenTypeName(tokens[i].type), tokens[i].text);
    }

    Parser parser;
    initParser(&parser, tokens, tokenCount);

    if (!expect(&parser, TOKEN_EQUAL)) return;

    ASTNode *root = parseComparison(&parser);
    if (root == NULL) {
        printf("\nParsing failed.\n");
        return;
    }

    printf("\nAST:\n\n");
    printAST(root, 0);

    printf("\nSemantic Analysis:\n\n");
    SemanticResult sem = analyzeAST(root);
    if (sem.valid) {
        printf("Semantic analysis successful.\n");
        printf("Result type: %s\n", semanticTypeName(sem.type));

        char normalized[1024];
        if (normalizeAST(root, cell, normalized, sizeof(normalized))) {
            printf("\nNormalized Structural Representation:\n\n");
            printf("%s\n", normalized);
        }
    } else {
        printf("Semantic error: %s\n", semanticErrorMessage(sem.error));
        printf("Error type: %s\n", semanticErrorName(sem.error));
    }

    freeAST(root);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        auditSpreadsheet(argv[1]);
    } else {
        runSingleFormulaDemo();
        printf("\nTip: Run 'formulaguard.exe <path_to_spreadsheet.xlsx>' to audit spreadsheets.\n");
    }
    return 0;
}