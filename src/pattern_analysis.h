#ifndef PATTERN_ANALYSIS_H
#define PATTERN_ANALYSIS_H

#include <stddef.h>

#define MAX_PATTERN_LEN 1024
#define MAX_CELL_LEN 16
#define MAX_FORMULA_LEN 128

/* A single formula record belonging to a group */
typedef struct {
    char cell[MAX_CELL_LEN];
    char formulaText[MAX_FORMULA_LEN];
} FormulaEntry;

/* A group of formulas sharing the exact same canonical pattern */
typedef struct {
    char pattern[MAX_PATTERN_LEN];
    FormulaEntry *formulas;
    int count;
    int capacity;
} PatternGroup;

/* Collection of pattern groups across a spreadsheet or selection */
typedef struct {
    PatternGroup *groups;
    int groupCount;
    int groupCapacity;
} PatternGroupCollection;

/*
 * Lifecycle functions
 */
void initGroupCollection(PatternGroupCollection *collection);
void freeGroupCollection(PatternGroupCollection *collection);

/*
 * Add a formula with its normalized pattern into the group collection.
 * If the pattern already exists, appends the formula location to that group.
 * If not, creates a new group.
 *
 * Returns 1 on success, 0 on failure (e.g. invalid arguments or allocation failure).
 */
int addFormulaToGroups(
    PatternGroupCollection *collection,
    const char *pattern,
    const char *cell,
    const char *formulaText
);

/*
 * Find an existing group by pattern string.
 * Returns pointer to PatternGroup, or NULL if not found.
 */
PatternGroup *findPatternGroup(
    const PatternGroupCollection *collection,
    const char *pattern
);

/*
 * Helper to parse, semantically validate, normalize, and add a formula to groups.
 * Returns 1 on success, 0 on error (lexer, parser, semantic, or normalization failure).
 */
int processAndGroupFormula(
    PatternGroupCollection *collection,
    const char *cell,
    const char *formulaText
);

/*
 * Display summary of all discovered pattern groups to stdout.
 */
void printGroupCollection(const PatternGroupCollection *collection);

#endif /* PATTERN_ANALYSIS_H */
