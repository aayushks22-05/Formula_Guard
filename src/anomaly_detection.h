#ifndef ANOMALY_DETECTION_H
#define ANOMALY_DETECTION_H

#include "pattern_analysis.h"

/*
 * Configuration thresholds for anomaly detection in a formula region.
 *
 * minRegionSize: Minimum total formulas required to establish a pattern baseline (e.g., 3 or 4).
 * dominantFrequencyThreshold: Minimum fraction of total formulas the dominant pattern
 *                            must represent to establish dominance (e.g., 0.60 for 60%).
 * outlierMaxCount: Maximum formula count for a group to be considered an isolated outlier (e.g., 1).
 */
typedef struct {
    int minRegionSize;
    double dominantFrequencyThreshold;
    int outlierMaxCount;
} AnomalyConfig;

/* Result classification for the region as a whole */
typedef enum {
    REGION_NORMAL,                /* All formulas share the same pattern */
    REGION_ANOMALIES_DETECTED,    /* One or more isolated outlier formulas detected */
    REGION_NO_DOMINANT_PATTERN,   /* Multiple competitive patterns, no clear dominant baseline */
    REGION_INSUFFICIENT_EVIDENCE  /* Not enough formulas to establish a baseline */
} RegionStatus;

/* Details for a single flagged formula */
typedef struct {
    char cell[MAX_CELL_LEN];
    char formulaText[MAX_FORMULA_LEN];
    char observedPattern[MAX_PATTERN_LEN];
    char dominantPattern[MAX_PATTERN_LEN];
    char reason[256];
    char suggestedRepair[MAX_FORMULA_LEN];
    double confidence; /* Percentage (e.g. 80.0) based on dominant group share */
} FlaggedAnomaly;

/* Complete report for an analyzed region */
typedef struct {
    RegionStatus status;
    int totalFormulas;
    const PatternGroup *dominantGroup;
    FlaggedAnomaly *anomalies;
    int anomalyCount;
    int anomalyCapacity;
} AnomalyReport;

/* Default configuration: min 4 formulas, 60% dominance threshold, max 1 occurrence for outlier */
AnomalyConfig defaultAnomalyConfig(void);

/* Lifecycle functions */
void initAnomalyReport(AnomalyReport *report);
void freeAnomalyReport(AnomalyReport *report);

/*
 * Analyze a collection of pattern groups for structural anomalies.
 * Populates report with status, dominant group, and any flagged formulas.
 */
void detectAnomalies(
    const PatternGroupCollection *collection,
    const AnomalyConfig *config,
    AnomalyReport *report
);

/* Print a formatted human-readable report */
void printAnomalyReport(const AnomalyReport *report);

/* Print rich structured region report with distribution tables and repair advice */
void printStructuredRegionReport(
    const char *regionName,
    const PatternGroupCollection *collection,
    const AnomalyReport *report
);

#endif /* ANOMALY_DETECTION_H */
