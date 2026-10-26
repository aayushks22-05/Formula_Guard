#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"


const char *tokenTypeName(TokenType type) {

    switch (type) {

        case TOKEN_EQUAL:
            return "EQUAL";

        case TOKEN_CELL:
            return "CELL";

        case TOKEN_NUMBER:
            return "NUMBER";

        case TOKEN_PLUS:
            return "PLUS";

        case TOKEN_MINUS:
            return "MINUS";

        case TOKEN_MULTIPLY:
            return "MULTIPLY";

        case TOKEN_DIVIDE:
            return "DIVIDE";

        case TOKEN_FUNCTION:
            return "FUNCTION";

        case TOKEN_LPAREN:
            return "LPAREN";

        case TOKEN_RPAREN:
            return "RPAREN";

        case TOKEN_COLON:
            return "COLON";

        case TOKEN_LESS:
            return "LESS";

        case TOKEN_GREATER:
            return "GREATER";

        case TOKEN_LESS_EQUAL:
            return "LESS_EQUAL";

        case TOKEN_GREATER_EQUAL:
            return "GREATER_EQUAL";

        case TOKEN_NOT_EQUAL:
            return "NOT_EQUAL";

        case TOKEN_COMMA:
            return "COMMA";

        case TOKEN_EOF:
            return "EOF";

        default:
            return "UNKNOWN";
    }
}


void addToken(Token tokens[], int *tokenCount,
              TokenType type, const char *text) {

    tokens[*tokenCount].type = type;

    strncpy(tokens[*tokenCount].text, text, sizeof(tokens[*tokenCount].text) - 1);
    tokens[*tokenCount].text[sizeof(tokens[*tokenCount].text) - 1] = '\0';

    (*tokenCount)++;
}


int lex(const char *formula, Token tokens[]) {

    int tokenCount = 0;


    while (*formula != '\0') {

        /* Ignore whitespace */
        if (isspace(*formula)) {

            formula++;
        }


        /* Equal */
        else if (*formula == '=') {

            addToken(tokens, &tokenCount,
                     TOKEN_EQUAL, "=");

            formula++;
        }


        /* Cell ($A$1, $A1, A$1, A1) or function (SUM(, IF() */
        else if (isalpha(*formula) || *formula == '$') {

            char text[50];
            size_t i = 0;
            int hasColDollar = 0;

            if (*formula == '$') {
                hasColDollar = 1;
                text[i++] = *formula++;
            }

            while (isalpha(*formula)) {
                if (i < sizeof(text) - 1) {
                    text[i++] = *formula;
                }
                formula++;
            }

            /* Optional $ before row number */
            if (*formula == '$') {
                if (i < sizeof(text) - 1) {
                    text[i++] = *formula;
                }
                formula++;
            }

            /* Cell: A1, $A1, A$1, $A$1 */
            if (isdigit(*formula)) {

                while (isdigit(*formula)) {
                    if (i < sizeof(text) - 1) {
                        text[i++] = *formula;
                    }
                    formula++;
                }

                text[i] = '\0';

                addToken(tokens, &tokenCount,
                         TOKEN_CELL, text);
            }
            /* Function: SUM(, AVERAGE(, etc. (only if no leading '$') */
            else if (!hasColDollar && *formula == '(') {

                text[i] = '\0';

                addToken(tokens, &tokenCount,
                         TOKEN_FUNCTION, text);
            }
            else {

                printf("Lexer Error: invalid identifier\n");

                return -1;
            }
        }


        /* Number */
        else if (isdigit(*formula)) {

            char text[50];
            size_t i = 0;


            while (isdigit(*formula)) {

                if (i < sizeof(text) - 1) {
                    text[i] = *formula;
                    i++;
                }
                formula++;
            }


            /* Decimal number */
            if (*formula == '.') {

                if (i < sizeof(text) - 1) {
                    text[i] = '.';
                    i++;
                }
                formula++;


                while (isdigit(*formula)) {

                    if (i < sizeof(text) - 1) {
                        text[i] = *formula;
                        i++;
                    }
                    formula++;
                }
            }


            text[i] = '\0';

            addToken(tokens, &tokenCount,
                     TOKEN_NUMBER, text);
        }


        /* Plus */
        else if (*formula == '+') {

            addToken(tokens, &tokenCount,
                     TOKEN_PLUS, "+");

            formula++;
        }


        /* Minus */
        else if (*formula == '-') {

            addToken(tokens, &tokenCount,
                     TOKEN_MINUS, "-");

            formula++;
        }


        /* Multiply */
        else if (*formula == '*') {

            addToken(tokens, &tokenCount,
                     TOKEN_MULTIPLY, "*");

            formula++;
        }


        /* Divide */
        else if (*formula == '/') {

            addToken(tokens, &tokenCount,
                     TOKEN_DIVIDE, "/");

            formula++;
        }


        /* Left parenthesis */
        else if (*formula == '(') {

            addToken(tokens, &tokenCount,
                     TOKEN_LPAREN, "(");

            formula++;
        }


        /* Right parenthesis */
        else if (*formula == ')') {

            addToken(tokens, &tokenCount,
                     TOKEN_RPAREN, ")");

            formula++;
        }


        /* Colon */
        else if (*formula == ':') {

            addToken(tokens, &tokenCount,
                     TOKEN_COLON, ":");

            formula++;
        }

        /* Comma */

        else if (*formula == ',') {

            addToken(tokens, &tokenCount,
                    TOKEN_COMMA, ",");

            formula++;
        }


        /* Less than / less than or equal / not equal */
        else if (*formula == '<') {

            if (*(formula + 1) == '=') {

                addToken(tokens, &tokenCount,
                         TOKEN_LESS_EQUAL, "<=");

                formula += 2;
            }

            else if (*(formula + 1) == '>') {

                addToken(tokens, &tokenCount,
                         TOKEN_NOT_EQUAL, "<>");

                formula += 2;
            }

            else {

                addToken(tokens, &tokenCount,
                         TOKEN_LESS, "<");

                formula++;
            }
        }


        /* Greater than / greater than or equal */
        else if (*formula == '>') {

            if (*(formula + 1) == '=') {

                addToken(tokens, &tokenCount,
                         TOKEN_GREATER_EQUAL, ">=");

                formula += 2;
            }

            else {

                addToken(tokens, &tokenCount,
                         TOKEN_GREATER, ">");

                formula++;
            }
        }


        /* Invalid character */
        else {

            printf("Lexer Error: unexpected character '%c'\n",
                   *formula);

            return -1;
        }
    }
    addToken(tokens, &tokenCount,
         TOKEN_EOF, "");

    return tokenCount;
}