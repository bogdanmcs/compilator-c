#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Token.h"
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"

int main()
{
    // lexical analyzer
    Token *tokens = analyzeLex("9.c");  // ("tests/x.c"), x = 0-9

    // syntax analyzer
    analyzeSyntax(tokens);

    //
    free(tokens);
    return 0;
}