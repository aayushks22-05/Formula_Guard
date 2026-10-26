#include <stdio.h>
#include "../src/lexer.h"

int main() {

    const char *formula = "=A1+B2*10";

    Token tokens[100];

    int tokenCount = lex(formula, tokens);

    printf("Token count: %d\n", tokenCount);

    for (int i = 0; i < tokenCount; i++) {

        printf("%s -> %s\n",
               tokenTypeName(tokens[i].type),
               tokens[i].text);
    }

    return 0;
}