#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pattern_analysis.h"

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
    printf("FormulaGuard Pattern Grouping Tests\n");
    printf("========================================\n\n");

    /* Test 1: Empty collection */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);
        assertTest("Empty collection has 0 groups", col.groupCount == 0);
        freeGroupCollection(&col);
    }

    /* Test 2: Three identical normalized patterns -> One group with 3 formulas */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        int ok1 = processAndGroupFormula(&col, "D2", "=B2+C2");
        int ok2 = processAndGroupFormula(&col, "D3", "=B3+C3");
        int ok3 = processAndGroupFormula(&col, "D5", "=B5+C5");

        assertTest("All 3 formulas processed successfully", ok1 && ok2 && ok3);
        assertTest("Three identical patterns form exactly 1 group", col.groupCount == 1);
        if (col.groupCount == 1) {
            assertTest("Group contains 3 formulas", col.groups[0].count == 3);
            assertTest("Group pattern is ADD(REL[-2,0], REL[-1,0])",
                       strcmp(col.groups[0].pattern, "ADD(REL[-2,0], REL[-1,0])") == 0);
            assertTest("First formula cell is D2", strcmp(col.groups[0].formulas[0].cell, "D2") == 0);
            assertTest("Second formula cell is D3", strcmp(col.groups[0].formulas[1].cell, "D3") == 0);
            assertTest("Third formula cell is D5", strcmp(col.groups[0].formulas[2].cell, "D5") == 0);
        }

        freeGroupCollection(&col);
    }

    /* Test 3: Two different patterns -> Two groups (from user prompt example) */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        processAndGroupFormula(&col, "D2", "=B2+C2");
        processAndGroupFormula(&col, "D3", "=B3+C3");
        processAndGroupFormula(&col, "D4", "=B4-C4");
        processAndGroupFormula(&col, "D5", "=B5+C5");

        assertTest("4 formulas partitioned into 2 groups (3 ADD, 1 SUB)", col.groupCount == 2);

        PatternGroup *grpAdd = findPatternGroup(&col, "ADD(REL[-2,0], REL[-1,0])");
        PatternGroup *grpSub = findPatternGroup(&col, "SUB(REL[-2,0], REL[-1,0])");

        assertTest("ADD group found", grpAdd != NULL);
        assertTest("SUB group found", grpSub != NULL);

        if (grpAdd && grpSub) {
            assertTest("ADD group has 3 formulas", grpAdd->count == 3);
            assertTest("SUB group has 1 formula", grpSub->count == 1);
            assertTest("SUB formula is at cell D4", strcmp(grpSub->formulas[0].cell, "D4") == 0);
        }

        freeGroupCollection(&col);
    }

    /* Test 4: Invalid formulas / normalization failure rejection */
    {
        PatternGroupCollection col;
        initGroupCollection(&col);

        int res1 = processAndGroupFormula(&col, "D2", "=B2+"); /* syntax error */
        int res2 = processAndGroupFormula(&col, "D2", "=B0+C1"); /* invalid cell row 0 */
        int res3 = processAndGroupFormula(&col, "D2", "=SUM(A1:B5,)"); /* bad syntax */

        assertTest("Syntax error formula rejected (returns 0)", res1 == 0);
        assertTest("Semantic error formula rejected (returns 0)", res2 == 0);
        assertTest("Trailing comma formula rejected (returns 0)", res3 == 0);
        assertTest("No groups created for rejected formulas", col.groupCount == 0);

        freeGroupCollection(&col);
    }

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    return failed > 0 ? 1 : 0;
}
