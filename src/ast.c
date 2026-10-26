#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"


ASTNode *createCellNode(const char *value) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }


    node->type = AST_CELL;

    strcpy(node->value, value);

    node->left = NULL;

    node->right = NULL;

    node->children = NULL;
    node->childCount = 0;


    return node;
}


ASTNode *createNumberNode(const char *value) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }


    node->type = AST_NUMBER;

    strcpy(node->value, value);

    node->left = NULL;

    node->right = NULL;

    node->children = NULL;
    node->childCount = 0;


    return node;
}


ASTNode *createBinaryOpNode(const char *operator,ASTNode *left,ASTNode *right) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }


    node->type = AST_BINARY_OP;

    strcpy(node->value, operator);

    node->left = left;

    node->right = right;


    return node;
}

ASTNode *createFunctionNode(
    const char *functionName,
    ASTNode **arguments,
    int argumentCount
) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {

        printf("AST Error: memory allocation failed\n");

        return NULL;
    }

    node->type = AST_FUNCTION;

    strcpy(node->value, functionName);

    node->left = NULL;

    node->right = NULL;

    node->children = arguments;

    node->childCount = argumentCount;

    return node;
}

ASTNode *createRangeNode(
    ASTNode *start,
    ASTNode *end
) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {

        printf("AST Error: memory allocation failed\n");

        return NULL;
    }

    node->type = AST_RANGE;

    node->value[0] = '\0';

    node->left = start;

    node->right = end;

    node->children = NULL;

    node->childCount = 0;

    return node;
}

void printAST(ASTNode *node, int depth) {

    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    if (node->type == AST_CELL) {
        printf("CELL: %s\n", node->value);
    }

    else if (node->type == AST_NUMBER) {
        printf("NUMBER: %s\n", node->value);
    }

    else if (node->type == AST_BINARY_OP) {
        printf("OPERATOR: %s\n", node->value);
    }

   
    else if (node->type == AST_FUNCTION) {

        printf("FUNCTION: %s\n", node->value);

        for (int i = 0; i < node->childCount; i++) {

            printAST(
                node->children[i],
                depth + 1
            );
        }

        return;
    }
    else if (node->type == AST_RANGE) {

        printf("RANGE\n");
    }

    printAST(node->left, depth + 1);

    printAST(node->right, depth + 1);
}

static void printNodeLabel(ASTNode *node) {
    if (node == NULL) {
        printf("(null)\n");
        return;
    }

    switch (node->type) {
        case AST_CELL:
            printf("CELL: %s\n", node->value);
            break;
        case AST_NUMBER:
            printf("NUMBER: %s\n", node->value);
            break;
        case AST_BINARY_OP:
            printf("OP: %s\n", node->value);
            break;
        case AST_FUNCTION:
            printf("FN: %s()\n", node->value);
            break;
        case AST_RANGE:
            printf("RANGE (:)\n");
            break;
        default:
            printf("NODE: %s\n", node->value);
            break;
    }
}

static void prettyPrintHelper(ASTNode *node, const char *prefix, int isLast) {
    if (node == NULL) {
        return;
    }

    printf("%s", prefix);
    printf("%s", isLast ? "`-- " : "|-- ");
    printNodeLabel(node);

    char nextPrefix[256];
    snprintf(nextPrefix, sizeof(nextPrefix), "%s%s", prefix, isLast ? "    " : "|   ");

    if (node->type == AST_FUNCTION) {
        for (int i = 0; i < node->childCount; i++) {
            prettyPrintHelper(node->children[i], nextPrefix, i == node->childCount - 1);
        }
    } else {
        int hasLeft = (node->left != NULL);
        int hasRight = (node->right != NULL);

        if (hasLeft && hasRight) {
            prettyPrintHelper(node->left, nextPrefix, 0);
            prettyPrintHelper(node->right, nextPrefix, 1);
        } else if (hasLeft) {
            prettyPrintHelper(node->left, nextPrefix, 1);
        } else if (hasRight) {
            prettyPrintHelper(node->right, nextPrefix, 1);
        }
    }
}

void prettyPrintAST(ASTNode *node) {
    if (node == NULL) {
        printf("(empty AST)\n");
        return;
    }

    printNodeLabel(node);

    if (node->type == AST_FUNCTION) {
        for (int i = 0; i < node->childCount; i++) {
            prettyPrintHelper(node->children[i], "", i == node->childCount - 1);
        }
    } else {
        int hasLeft = (node->left != NULL);
        int hasRight = (node->right != NULL);

        if (hasLeft && hasRight) {
            prettyPrintHelper(node->left, "", 0);
            prettyPrintHelper(node->right, "", 1);
        } else if (hasLeft) {
            prettyPrintHelper(node->left, "", 1);
        } else if (hasRight) {
            prettyPrintHelper(node->right, "", 1);
        }
    }
}

void freeAST(ASTNode *node) {

    if (node == NULL) {
        return;
    }

    if (node->type == AST_FUNCTION && node->children != NULL) {
        for (int i = 0; i < node->childCount; i++) {
            freeAST(node->children[i]);
        }
        free(node->children);
    }

    freeAST(node->left);

    freeAST(node->right);

    free(node);
}