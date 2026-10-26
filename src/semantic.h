#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef enum {

    SEM_ERROR_NONE,

    SEM_ERROR_INVALID_CELL,

    SEM_ERROR_INVALID_RANGE,

    SEM_ERROR_INVALID_OPERATOR,

    SEM_ERROR_UNKNOWN_FUNCTION,

    SEM_ERROR_INVALID_ARGUMENT_COUNT,

    SEM_ERROR_INVALID_ARGUMENT_TYPE

} SemanticErrorType;

typedef enum {

    SEM_NUMBER,

    SEM_CELL,

    SEM_RANGE,

    SEM_BOOLEAN,

    SEM_ERROR

} SemanticType;


typedef struct {

    int valid;

    SemanticType type;

    SemanticErrorType error;

} SemanticResult;


SemanticResult analyzeAST(ASTNode *node);

const char *semanticTypeName(SemanticType type);

int isScalarType(SemanticType type);

const char *semanticErrorName(SemanticErrorType error);

const char *semanticErrorMessage(
    SemanticErrorType error
);
#endif