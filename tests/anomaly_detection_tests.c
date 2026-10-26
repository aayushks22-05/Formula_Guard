#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pattern_analysis.h"
#include "../src/anomaly_detection.h"

static int passed = 0;
static int failed = 0;

static void assertTest(const char *name, int condition) {
    if (condition) {
        printf("[PASS] %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s\n", name);
        failed++;
    }
}

int main(void) {
    printf("========================================\n");
    printf("FormulaGuard Anomaly Detection Tests\n");
    printf("========================================\n\n");

    /*
     * Test Case 1: 4 formulas with 3 ADD patterns and 1 SUB pattern
     * Cell D2 =B2+C2, D3 =B3+C3, D4 =B4-C4 (outlier), D5 =B5+C5
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4-C4");
        processAndGroupFormula(&col, "D5", "=B5+C5");

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        assertTest("Case 1: Status is REGION_ANOMALIES_DETECTED", report.status == REGION_ANOMALIES_DETECTED);
        assertTest("Case 1: Total formulas is 4", report.totalFormulas == 4);
        assertTest("Case 1: Exactly 1 anomaly flagged", report.anomalyCount == 1);
        if (report.anomalyCount == 1) {
            assertTest("Case 1: Flagged cell is D4", strcmp(report.anomalies[0].cell, "D4") == 0);
            assertTest("Case 1: Observed pattern is SUB", strstr(report.anomalies[0].observedPattern, "SUB") != NULL);
            assertTest("Case 1: Dominant pattern is ADD", strstr(report.anomalies[0].dominantPattern, "ADD") != NULL);
            assertTest("Case 1: Reason is reported", strlen(report.anomalies[0].reason) > 0);
        }

        printAnomalyReport(&report);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    /*
     * Test Case 2: Region where every formula has the same pattern (4 formulas)
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4+C4");
        processAndGroupFormula(&col, "D5", "=B5+C5");

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        assertTest("Case 2: Status is REGION_NORMAL", report.status == REGION_NORMAL);
        assertTest("Case 2: 0 anomalies flagged", report.anomalyCount == 0);
        assertTest("Case 2: Dominant group is ADD", report.dominantGroup != NULL && strstr(report.dominantGroup->pattern, "ADD") != NULL);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    /*
     * Test Case 3: Two equally common patterns (e.g. 2 ADD vs 2 SUB)
     * Avoid automatically declaring a winner or outlier
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4-C4");
        processAndGroupFormula(&col, "D5", "=B5-C5");

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        assertTest("Case 3: Status is REGION_NO_DOMINANT_PATTERN", report.status == REGION_NO_DOMINANT_PATTERN);
        assertTest("Case 3: 0 anomalies flagged (avoids premature declaration)", report.anomalyCount == 0);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    /*
     * Test Case 4: Small region with insufficient evidence (< minRegionSize = 4)
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3-C3");

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        assertTest("Case 4: Status is REGION_INSUFFICIENT_EVIDENCE", report.status == REGION_INSUFFICIENT_EVIDENCE);
        assertTest("Case 4: 0 anomalies flagged", report.anomalyCount == 0);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    /*
     * Test Case 5: Formula rejected by parser/lexer does not corrupt group count or detection
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4+C4");
        int resBad = processAndGroupFormula(&col, "D5", "=B5+"); /* syntax error */

        assertTest("Case 5: Bad formula correctly rejected by frontend", resBad == 0);

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        /* Only 3 valid formulas remain -> insufficient evidence (< 4) */
        assertTest("Case 5: Region falls back to INSUFFICIENT_EVIDENCE", report.status == REGION_INSUFFICIENT_EVIDENCE);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    /*
     * Test Case 6: Multiple distinct outliers in a large region
     * 5 ADD, 1 SUB at D7, 1 MUL at D8
     */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4+C4");
        processAndGroupFormula(&col, "D5", "=B5+C5");
        processAndGroupFormula(&col, "D6", "=B6+C6");
        processAndGroupFormula(&col, "D7", "=B7-C7"); /* Outlier 1: SUB */
        processAndGroupFormula(&col, "D8", "=B8*C8"); /* Outlier 2: MUL */

        AnomalyReport report;
        detectAnomalies(&col, NULL, &report);

        assertTest("Case 6: Status is REGION_ANOMALIES_DETECTED", report.status == REGION_ANOMALIES_DETECTED);
        assertTest("Case 6: Total formulas is 7", report.totalFormulas == 7);
        assertTest("Case 6: Both outliers flagged (count == 2)", report.anomalyCount == 2);

        int flaggedD7 = 0;
        int flaggedD8 = 0;
        for (int i = 0; i < report.anomalyCount; i++) {
            if (strcmp(report.anomalies[i].cell, "D7") == 0) flaggedD7 = 1;
            if (strcmp(report.anomalies[i].cell, "D8") == 0) flaggedD8 = 1;
        }
        assertTest("Case 6: Outlier D7 correctly flagged", flaggedD7);
        assertTest("Case 6: Outlier D8 correctly flagged", flaggedD8);

        freeAnomalyReport(&report);
        freeGroupCollection(&col);
    }

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    return failed > 0 ? 1 : 0;
}
