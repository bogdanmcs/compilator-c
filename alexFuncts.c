#include "alexFuncts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char *createString(const char *strInt, char *strEnd)
{
    char *newText;
    int i, n = strEnd - strInt;
    newText = malloc (sizeof(char) * n);
    int textIndex = 0;
    int k = 0;
    if(strInt[0] == '\"') k = 1;
    for(i = k; i < n - k; i++)
    {
        if(strInt[i] == '\\'){
            if(strInt[i+1]=='a'){
                newText[textIndex++] = '\a'; i++;
            } else if(strInt[i+1]=='b'){
                newText[textIndex++] = '\b'; i++;
            } else if(strInt[i+1]=='f'){
                newText[textIndex++] = '\f'; i++;
            } else if(strInt[i+1]=='n'){
                newText[textIndex++] = '\n'; i++;
            } else if(strInt[i+1]=='r'){
                newText[textIndex++] = '\r'; i++;
            } else if(strInt[i+1]=='t'){
                newText[textIndex++] = '\t'; i++;
            } else if(strInt[i+1]=='v'){
                newText[textIndex++] = '\v'; i++;
            } else if(strInt[i+1]=='\''){
                newText[textIndex++] = '\''; i++;
            } else if(strInt[i+1]=='?'){
                newText[textIndex++] = '\?'; i++;
            } else if(strInt[i+1]=='\"'){
                newText[textIndex++] = '\"'; i++;
            } else if(strInt[i+1]=='\\'){
                newText[textIndex++] = '\\'; i++;
            } else if(strInt[i+1]=='0'){
                newText[textIndex++] = '\0'; i++;
            } else {
                printf("Error: cannot build CT_STRING, invalid character[]\n");
                exit(7);
            }
        } else newText[textIndex++] = strInt[i];
    }
    newText[i] = '\0';
    return newText;
}

double createDouble(const char *strInt, char *strEnd)
{
    char *newText;
    int i, n = strEnd - strInt;
    newText = malloc (sizeof(char) * n);
    for(i = 0; i < n; i++)
        newText[i] = strInt[i];
    newText[i] = '\0';
    double textToDouble = atof(newText);
    return textToDouble;
}

long int createLongInt(const char *strInt, char *strEnd)
{
    char *newText;
    int i, n = strEnd - strInt;
    newText = malloc (sizeof(char) * n);
    for(i = 0; i < n; i++)
        newText[i] = strInt[i];
    newText[i] = '\0';
    long int textToLongInt = strtol(newText, NULL, 0);
    return textToLongInt;
}

long int createLongIntChar(const char *strInt, char *strEnd)
{
    char newChar;
    int n = strEnd - strInt;
    if(n == 3) newChar = strInt[1];
    else if(n == 4){
        if(strInt[2] == 'a') newChar = '\a';
        else if(strInt[2] == 'b') newChar = '\b'; else if(strInt[2] == 'f') newChar = '\f'; else if(strInt[2] == 'n') newChar = '\n';
        else if(strInt[2] == 'r') newChar = '\r'; else if(strInt[2] == 't') newChar = '\t'; else if(strInt[2] == 'v') newChar = '\v';
        else if(strInt[2] == '\'') newChar = '\''; else if(strInt[2] == '?') newChar = '\?'; else if(strInt[2] == '\"') newChar = '\"';
        else if(strInt[2] == '\\') newChar = '\\'; else if(strInt[2] == '0') newChar = '\0';
    }

    long int textToLongIntChar = newChar;
    return textToLongIntChar;
}
