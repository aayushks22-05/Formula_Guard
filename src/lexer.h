#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EQUAL,
    TOKEN_CELL,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_FUNCTION,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COLON,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_COMMA,
    TOKEN_EOF,
} TokenType;


typedef struct {
    TokenType type;
    char text[50];
} Token;


const char *tokenTypeName(TokenType type);


void addToken(Token tokens[], int *tokenCount,
              TokenType type, const char *text);


int lex(const char *formula, Token tokens[]);

#endif