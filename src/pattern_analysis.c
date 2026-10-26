#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pattern_analysis.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "normalize.h"

void initGroupCollection(PatternGroupCollection *collection) {
    if (collection == NULL) return;
    collection->groups = NULL;
    collection->groupCount = 0;
    collection->groupCapacity = 0;
}

void freeGroupCollection(PatternGroupCollection *collection) {
    if (collection == NULL) return;

    for (int i = 0; i < collection->groupCount; i++) {
        if (collection->groups[i].formulas != NULL) {
            free(collection->groups[i].formulas);
            collection->groups[i].formulas = NULL;
        }
    }

    if (collection->groups != NULL) {
        free(collection->groups);
        collection->groups = NULL;
    }

    collection->groupCount = 0;
    collection->groupCapacity = 0;
}

PatternGroup *findPatternGroup(
    const PatternGroupCollection *collection,
    const char *pattern
) {
    if (collection == NULL || pattern == NULL) {
        return NULL;
    }

    for (int i = 0; i < collection->groupCount; i++) {
        if (strcmp(collection->groups[i].pattern, pattern) == 0) {
            return &collection->groups[i];
        }
    }

    return NULL;
}

int addFormulaToGroups(
    PatternGroupCollection *collection,
    const char *pattern,
    const char *cell,
    const char *formulaText
) {
    if (collection == NULL || pattern == NULL || cell == NULL) {
        return 0;
    }

    PatternGroup *group = findPatternGroup(collection, pattern);

    /* If group doesn't exist, create and append a new group */
    if (group == NULL) {
        if (collection->groupCount >= collection->groupCapacity) {
            int newCap = collection->groupCapacity == 0 ? 4 : collection->groupCapacity * 2;
            PatternGroup *newGroups = (PatternGroup *)realloc(
                collection->groups,
                sizeof(PatternGroup) * newCap
            );
            if (newGroups == NULL) {
                return 0;
            }
            collection->groups = newGroups;
            collection->groupCapacity = newCap;
        }

        group = &collection->groups[collection->groupCount];
        strncpy(group->pattern, pattern, sizeof(group->pattern) - 1);
        group->pattern[sizeof(group->pattern) - 1] = '\0';
        group->formulas = NULL;
        group->count = 0;
        group->capacity = 0;
        collection->groupCount++;
    }

    /* Add the formula location to this group */
    if (group->count >= group->capacity) {
        int newCap = group->capacity == 0 ? 4 : group->capacity * 2;
        FormulaEntry *newFormulas = (FormulaEntry *)realloc(
            group->formulas,
            sizeof(FormulaEntry) * newCap
        );
        if (newFormulas == NULL) {
            return 0;
        }
        group->formulas = newFormulas;
        group->capacity = newCap;
    }

    FormulaEntry *entry = &group->formulas[group->count];
    strncpy(entry->cell, cell, sizeof(entry->cell) - 1);
    entry->cell[sizeof(entry->cell) - 1] = '\0';

    if (formulaText != NULL) {
        strncpy(entry->formulaText, formulaText, sizeof(entry->formulaText) - 1);
        entry->formulaText[sizeof(entry->formulaText) - 1] = '\0';
    } else {
        entry->formulaText[0] = '\0';
    }

    group->count++;
    return 1;
}

int processAndGroupFormula(
    PatternGroupCollection *collection,
    const char *cell,
    const char *formulaText
) {
    if (collection == NULL || cell == NULL || formulaText == NULL) {
        return 0;
    }

    Token tokens[100];
    int tokenCount = lex(formulaText, tokens);
    if (tokenCount < 0) {
        return 0;
    }

    Parser parser;
    initParser(&parser, tokens, tokenCount);

    if (!expect(&parser, TOKEN_EQUAL)) {
        return 0;
    }

    ASTNode *root = parseComparison(&parser);
    if (root == NULL) {
        return 0;
    }

    Token *trailing = peek(&parser);
    if (trailing != NULL && trailing->type != TOKEN_EOF) {
        freeAST(root);
        return 0;
    }

    SemanticResult sem = analyzeAST(root);
    if (!sem.valid) {
        freeAST(root);
        return 0;
    }

    char pattern[MAX_PATTERN_LEN];
    if (!normalizeAST(root, cell, pattern, sizeof(pattern))) {
        freeAST(root);
        return 0;
    }

    freeAST(root);

    return addFormulaToGroups(collection, pattern, cell, formulaText);
}

void printGroupCollection(const PatternGroupCollection *collection) {
    if (collection == NULL) return;

    printf("========================================\n");
    printf("Pattern Group Analysis Summary\n");
    printf("Total Groups: %d\n", collection->groupCount);
    printf("========================================\n\n");

    for (int i = 0; i < collection->groupCount; i++) {
        const PatternGroup *grp = &collection->groups[i];
        printf("Pattern Group %c (%d formula%s):\n",
               'A' + (i % 26),
               grp->count,
               grp->count == 1 ? "" : "s");
        printf("  Canonical: %s\n", grp->pattern);
        printf("  Locations: ");
        for (int j = 0; j < grp->count; j++) {
            if (grp->formulas[j].formulaText[0] != '\0') {
                printf("%s (%s)", grp->formulas[j].cell, grp->formulas[j].formulaText);
            } else {
                printf("%s", grp->formulas[j].cell);
            }
            if (j < grp->count - 1) printf(", ");
        }
        printf("\n\n");
    }
}
