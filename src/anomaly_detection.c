#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anomaly_detection.h"

AnomalyConfig defaultAnomalyConfig(void) {
    AnomalyConfig cfg;
    cfg.minRegionSize = 4;
    cfg.dominantFrequencyThreshold = 0.60;
    cfg.outlierMaxCount = 1;
    return cfg;
}

void initAnomalyReport(AnomalyReport *report) {
    if (report == NULL) return;
    report->status = REGION_INSUFFICIENT_EVIDENCE;
    report->totalFormulas = 0;
    report->dominantGroup = NULL;
    report->anomalies = NULL;
    report->anomalyCount = 0;
    report->anomalyCapacity = 0;
}

void freeAnomalyReport(AnomalyReport *report) {
    if (report == NULL) return;
    if (report->anomalies != NULL) {
        free(report->anomalies);
        report->anomalies = NULL;
    }
    report->anomalyCount = 0;
    report->anomalyCapacity = 0;
    report->dominantGroup = NULL;
}

static void suggestRepair(
    const PatternGroup *dominantGroup,
    char *outBuf,
    size_t outSize
) {
    if (dominantGroup == NULL || dominantGroup->count == 0 || outBuf == NULL || outSize == 0) {
        if (outBuf && outSize > 0) outBuf[0] = '\0';
        return;
    }

    /* Use the first formula in dominant group as template */
    const FormulaEntry *sample = &dominantGroup->formulas[0];
    const char *sampleForm = sample->formulaText;

    /*
     * If dominant is simple function like =SUM(...) and target has =COUNT(...),
     * or simple op replace:
     */
    if (strncmp(sampleForm, "=SUM(", 5) == 0 && outSize > 5) {
        strncpy(outBuf, "=SUM(", outSize - 1);
        outBuf[outSize - 1] = '\0';
        /* Extract arguments from sample or target if matching */
        const char *arg = strchr(sampleForm, '(');
        if (arg) {
            strncat(outBuf, arg + 1, outSize - strlen(outBuf) - 1);
        }
        return;
    }

    /* Fallback to sample formula representation */
    snprintf(outBuf, outSize, "%s (pattern: %s)", sampleForm, dominantGroup->pattern);
}

static int addFlaggedAnomaly(
    AnomalyReport *report,
    const char *cell,
    const char *formulaText,
    const char *observedPattern,
    const char *dominantPattern,
    const char *reason,
    const char *suggestedRepair,
    double confidence
) {
    if (report->anomalyCount >= report->anomalyCapacity) {
        int newCap = report->anomalyCapacity == 0 ? 4 : report->anomalyCapacity * 2;
        FlaggedAnomaly *newArr = (FlaggedAnomaly *)realloc(
            report->anomalies,
            sizeof(FlaggedAnomaly) * newCap
        );
        if (newArr == NULL) {
            return 0;
        }
        report->anomalies = newArr;
        report->anomalyCapacity = newCap;
    }

    FlaggedAnomaly *fa = &report->anomalies[report->anomalyCount];
    strncpy(fa->cell, cell, sizeof(fa->cell) - 1);
    fa->cell[sizeof(fa->cell) - 1] = '\0';

    if (formulaText != NULL) {
        strncpy(fa->formulaText, formulaText, sizeof(fa->formulaText) - 1);
        fa->formulaText[sizeof(fa->formulaText) - 1] = '\0';
    } else {
        fa->formulaText[0] = '\0';
    }

    strncpy(fa->observedPattern, observedPattern, sizeof(fa->observedPattern) - 1);
    fa->observedPattern[sizeof(fa->observedPattern) - 1] = '\0';

    strncpy(fa->dominantPattern, dominantPattern, sizeof(fa->dominantPattern) - 1);
    fa->dominantPattern[sizeof(fa->dominantPattern) - 1] = '\0';

    strncpy(fa->reason, reason, sizeof(fa->reason) - 1);
    fa->reason[sizeof(fa->reason) - 1] = '\0';

    if (suggestedRepair != NULL) {
        strncpy(fa->suggestedRepair, suggestedRepair, sizeof(fa->suggestedRepair) - 1);
        fa->suggestedRepair[sizeof(fa->suggestedRepair) - 1] = '\0';
    } else {
        fa->suggestedRepair[0] = '\0';
    }

    fa->confidence = confidence;

    report->anomalyCount++;
    return 1;
}

void detectAnomalies(
    const PatternGroupCollection *collection,
    const AnomalyConfig *config,
    AnomalyReport *report
) {
    if (report == NULL) return;
    initAnomalyReport(report);

    AnomalyConfig cfg = (config != NULL) ? *config : defaultAnomalyConfig();

    if (collection == NULL || collection->groupCount == 0) {
        report->status = REGION_INSUFFICIENT_EVIDENCE;
        return;
    }

    /* Calculate total formulas across all groups in this region */
    int totalFormulas = 0;
    for (int i = 0; i < collection->groupCount; i++) {
        totalFormulas += collection->groups[i].count;
    }
    report->totalFormulas = totalFormulas;

    /* Rule: Check if region has sufficient evidence */
    if (totalFormulas < cfg.minRegionSize) {
        report->status = REGION_INSUFFICIENT_EVIDENCE;
        return;
    }

    /* Rule: Single group containing all formulas -> completely normal */
    if (collection->groupCount == 1) {
        report->status = REGION_NORMAL;
        report->dominantGroup = &collection->groups[0];
        return;
    }

    /* Find the largest group */
    int maxCount = -1;
    int dominantIdx = -1;
    int secondMaxCount = -1;

    for (int i = 0; i < collection->groupCount; i++) {
        int cnt = collection->groups[i].count;
        if (cnt > maxCount) {
            secondMaxCount = maxCount;
            maxCount = cnt;
            dominantIdx = i;
        } else if (cnt > secondMaxCount) {
            secondMaxCount = cnt;
        }
    }

    const PatternGroup *dominant = &collection->groups[dominantIdx];
    double dominantRatio = (double)dominant->count / (double)totalFormulas;
    double confidencePercent = dominantRatio * 100.0;

    /*
     * Rule: Dominant group must meet frequency threshold (>= 60%)
     * AND be strictly greater than any second pattern (not a tie).
     */
    if (dominantRatio < cfg.dominantFrequencyThreshold || maxCount == secondMaxCount) {
        report->status = REGION_NO_DOMINANT_PATTERN;
        report->dominantGroup = NULL;
        return;
    }

    report->dominantGroup = dominant;

    /*
     * Smart Outlier Detection:
     * A minority pattern is flagged as an anomaly if:
     * 1. It is an isolated single occurrence (count <= outlierMaxCount, e.g. 1)
     * OR
     * 2. The dominant baseline is clearly established (>= 75.0% dominance)
     *    and this minority group represents a small minority (<= 25.0% of the region).
     *    (e.g., 8 vs 2 in a 10-formula series = 80% vs 20% -> Flagged for review!)
     */
    int anomaliesFound = 0;
    for (int i = 0; i < collection->groupCount; i++) {
        if (i == dominantIdx) continue;

        const PatternGroup *grp = &collection->groups[i];
        double grpShare = (double)grp->count / (double)totalFormulas;

        int isOutlier = 0;
        if (grp->count <= cfg.outlierMaxCount) {
            isOutlier = 1;
        } else if (dominantRatio >= 0.75 && grpShare <= 0.25) {
            isOutlier = 1;
        }

        if (isOutlier) {
            for (int j = 0; j < grp->count; j++) {
                char repBuf[MAX_FORMULA_LEN];
                suggestRepair(dominant, repBuf, sizeof(repBuf));

                addFlaggedAnomaly(
                    report,
                    grp->formulas[j].cell,
                    grp->formulas[j].formulaText,
                    grp->pattern,
                    dominant->pattern,
                    "Structural pattern differs from the dominant pattern.",
                    repBuf,
                    confidencePercent
                );
                anomaliesFound++;
            }
        }
    }

    if (anomaliesFound > 0) {
        report->status = REGION_ANOMALIES_DETECTED;
    } else {
        report->status = REGION_NO_DOMINANT_PATTERN;
    }
}

void printAnomalyReport(const AnomalyReport *report) {
    if (report == NULL) return;

    printf("========================================\n");
    printf("FormulaGuard Anomaly Report\n");
    printf("========================================\n");

    switch (report->status) {
        case REGION_NORMAL:
            printf("Status: NORMAL\n");
            printf("Total Formulas: %d\n", report->totalFormulas);
            if (report->dominantGroup) {
                printf("Established Pattern: %s\n", report->dominantGroup->pattern);
            }
            printf("Message: All formulas in region share identical structural pattern.\n\n");
            break;

        case REGION_INSUFFICIENT_EVIDENCE:
            printf("Status: INSUFFICIENT EVIDENCE\n");
            printf("Total Formulas: %d\n", report->totalFormulas);
            printf("Message: Region has too few formulas to establish a reliable baseline.\n\n");
            break;

        case REGION_NO_DOMINANT_PATTERN:
            printf("Status: NO DOMINANT PATTERN\n");
            printf("Total Formulas: %d\n", report->totalFormulas);
            printf("Message: Multiple structural patterns coexist without a clear dominant majority.\n");
            printf("         Avoiding declaring smaller groups anomalous.\n\n");
            break;

        case REGION_ANOMALIES_DETECTED:
            printf("Status: POTENTIAL ANOMALIES DETECTED\n");
            printf("Total Formulas: %d\n", report->totalFormulas);
            if (report->dominantGroup) {
                printf("Dominant Pattern: %s (%d occurrences)\n",
                       report->dominantGroup->pattern,
                       report->dominantGroup->count);
            }
            printf("Flagged Count: %d\n\n", report->anomalyCount);

            for (int i = 0; i < report->anomalyCount; i++) {
                const FlaggedAnomaly *fa = &report->anomalies[i];
                printf("----------------------------------------\n");
                printf("Cell: %s\n", fa->cell);
                if (fa->formulaText[0] != '\0') {
                    printf("Formula: %s\n", fa->formulaText);
                }
                printf("Observed pattern: %s\n", fa->observedPattern);
                printf("Dominant pattern: %s\n", fa->dominantPattern);
                printf("Reason: %s\n", fa->reason);
                printf("Status: Potential anomaly -- manual review recommended.\n");
            }
            printf("----------------------------------------\n\n");
            break;
    }
}

void printStructuredRegionReport(
    const char *regionName,
    const PatternGroupCollection *collection,
    const AnomalyReport *report
) {
    if (regionName == NULL || collection == NULL || report == NULL) return;

    printf("======================================================================\n");
    printf("REGION: %s (%d formulas, %d distinct pattern%s)\n",
           regionName, report->totalFormulas, collection->groupCount,
           collection->groupCount == 1 ? "" : "s");
    printf("======================================================================\n");

    /* 1. Print Pattern Distribution Table */
    printf("PATTERN DISTRIBUTION:\n");
    printf("  Rank  Pattern %-35s Count  Share    Classification\n", "");
    printf("  --------------------------------------------------------------------\n");

    for (int i = 0; i < collection->groupCount; i++) {
        const PatternGroup *grp = &collection->groups[i];
        double share = report->totalFormulas > 0 ? ((double)grp->count / report->totalFormulas) * 100.0 : 0.0;

        const char *classification = "[COEXISTING]";
        if (report->dominantGroup == grp) {
            classification = "[DOMINANT]";
        } else if (report->status == REGION_ANOMALIES_DETECTED &&
                   (grp->count <= 1 || (report->dominantGroup &&
                    ((double)report->dominantGroup->count / report->totalFormulas >= 0.75 &&
                     (double)grp->count / report->totalFormulas <= 0.25)))) {
            classification = "[OUTLIER]";
        } else if (report->status == REGION_NORMAL) {
            classification = "[ESTABLISHED]";
        }

        /* Format truncated pattern for neat tabular alignment */
        char shortPat[42];
        if (strlen(grp->pattern) > 40) {
            strncpy(shortPat, grp->pattern, 37);
            shortPat[37] = '\0';
            strcat(shortPat, "...");
        } else {
            strncpy(shortPat, grp->pattern, sizeof(shortPat) - 1);
            shortPat[sizeof(shortPat) - 1] = '\0';
        }

        printf("  #%-4d %-42s %-6d %5.1f%%   %-14s\n",
               i + 1, shortPat, grp->count, share, classification);

        /* Print member cells */
        printf("        Cells: ");
        for (int c = 0; c < grp->count; c++) {
            printf("%s", grp->formulas[c].cell);
            if (c < grp->count - 1) printf(", ");
            if (c > 0 && c % 12 == 0 && c < grp->count - 1) printf("\n               ");
        }
        printf("\n");
    }
    printf("  --------------------------------------------------------------------\n\n");

    /* 2. Status verdict & Actionable findings */
    switch (report->status) {
        case REGION_NORMAL:
            printf("  STATUS: NORMAL\n");
            printf("  All formulas in %s follow the established structural pattern.\n\n", regionName);
            break;

        case REGION_INSUFFICIENT_EVIDENCE:
            printf("  STATUS: INSUFFICIENT EVIDENCE\n");
            printf("  Region has too few formulas to establish a statistically reliable baseline.\n\n");
            break;

        case REGION_NO_DOMINANT_PATTERN:
            printf("  STATUS: CONTESTED / NO DOMINANT PATTERN\n");
            printf("  Multiple formula patterns share this region without a clear dominant majority.\n");
            printf("  Review whether these different clusters represent distinct calculation purposes.\n\n");
            break;

        case REGION_ANOMALIES_DETECTED:
            printf("  STATUS: %d POTENTIAL ANOMAL%s DETECTED\n",
                   report->anomalyCount, report->anomalyCount == 1 ? "Y" : "IES");
            printf("  Dominant Pattern Baseline: %s (%.1f%% confidence)\n\n",
                   report->dominantGroup ? report->dominantGroup->pattern : "N/A",
                   report->dominantGroup ? ((double)report->dominantGroup->count / report->totalFormulas) * 100.0 : 0.0);

            for (int a = 0; a < report->anomalyCount; a++) {
                const FlaggedAnomaly *fa = &report->anomalies[a];
                printf("  [OUTLIER #%d] Cell %s\n", a + 1, fa->cell);
                if (fa->formulaText[0] != '\0') {
                    printf("    - Current Formula:   %s\n", fa->formulaText);
                }
                printf("    - Observed Pattern:  %s\n", fa->observedPattern);
                printf("    - Expected Pattern:  %s\n", fa->dominantPattern);
                printf("    - Confidence:        %.1f%% (Based on %d surrounding cells)\n",
                       fa->confidence, report->dominantGroup ? report->dominantGroup->count : 0);
                printf("    - Reason:            %s\n", fa->reason);
                printf("    - Status:            Potential anomaly -- manual review recommended.\n\n");
            }
            break;
    }
}
