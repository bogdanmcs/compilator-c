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
    Token *tokens = analyzeLex("9.c");

    // syntax analyzer
    analyzeSyntax(tokens);

    //
    free(tokens);
    return 0;

}