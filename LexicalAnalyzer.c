#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "Token.h"
#include "ErrChecker.h"
#include "LexicalAnalyzer.h"

enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, MUL, SUB, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, END}; // codurile AL

const char* enumNames[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                           "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "COMMA",
                           "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC", "ADD", "MUL", "SUB",
                           "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER",
                           "GREATEREQ", "END"};

Token *tokens, *lastToken;
char *pCurrent;
int line;

char *createString(const char *strInt, char *strEnd)
{
    char *newText;
    int i, n = strEnd - strInt;
    newText = malloc (sizeof(char) * n);
    int textIndex = 0;
    int k = 0;
    if(strInt[0] == '\"') k = 1;
    for(i = k; i < n - k; i++) // i=1,i<n-1 => fara " la inceput si final
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
                printf("Eroare la construire CT_STRING, caracter invalid []\n");
                exit(7);
            }
        } else newText[textIndex++] = strInt[i];
    }
    newText[textIndex] = '\0';
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

void init()
{
    line = 1;
    tokens = malloc(sizeof(struct Token));
    tokens -> next = NULL;
    lastToken = NULL;
}

Token *addTk(int code, int line)
{
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    }else{
        tokens=tk;
    }
    lastToken=tk;
    return tk;
}

int getNextToken()
{
    int state = 0;
    int nCh;
    char ch;
    const char *pStart;
    Token *tk;

    while(1){
        ch = *pCurrent;
        //printf("ch=|%c|\n", ch);
        switch(state){
            case 0:
                if(isalpha(ch) || ch == '_'){
                    pStart = pCurrent;
                    pCurrent++;
                    state = 1;
                }
                else if(ch == ' ' || ch == '\r' || ch == '\t'){
                    pCurrent++;
                }
                else if(ch == '\n'){
                    line++;
                    pCurrent++;
                }
                else if(ch == ','){
                    pCurrent++;
                    state = 3;
                }
                else if(ch == ';'){
                    pCurrent++;
                    state = 4;
                }
                else if(ch == '('){
                    pCurrent++;
                    state = 5;
                }
                else if(ch == ')'){
                    pCurrent++;
                    state = 6;
                }
                else if(ch == '['){
                    pCurrent++;
                    state = 7;
                }
                else if(ch == ']'){
                    pCurrent++;
                    state = 8;
                }
                else if(ch == '{'){
                    pCurrent++;
                    state = 9;
                }
                else if(ch == '}'){
                    pCurrent++;
                    state = 10;
                }
                else if(ch == '.'){
                    pCurrent++;
                    state = 11;
                }
                else if(ch == '&'){
                    pCurrent++;
                    state = 12;
                }
                else if(ch == '|'){
                    pCurrent++;
                    state = 14;
                }
                else if(ch == '!'){
                    pCurrent++;
                    state = 16;
                }
                else if(ch == '='){
                    pCurrent++;
                    state = 19;
                }
                else if(ch == '<'){
                    pCurrent++;
                    state = 22;
                }
                else if(ch == '>'){
                    pCurrent++;
                    state = 25;
                }
                else if(ch == '+'){
                    pCurrent++;
                    state = 28;
                }
                else if(ch == '-'){
                    pCurrent++;
                    state = 29;
                }
                else if(ch == '*'){
                    pCurrent++;
                    state = 30;
                }
                else if(ch == '/'){
                    pCurrent++;
                    state = 31;
                }
                else if(isdigit(ch) && ch != '0'){
                    pStart = pCurrent;
                    pCurrent++;
                    state = 36;
                }
                else if(ch == '0'){
                    pStart = pCurrent;
                    pCurrent++;
                    state = 37;
                }
                else if(ch == '\''){
                    pStart = pCurrent;
                    pCurrent++;
                    state = 49;
                }
                else if(ch == '\"'){
                    pStart = pCurrent;
                    pCurrent++;
                    state = 53;
                }
                else if(ch == 0){
                    addTk(END, line);
                    return END;
                }
                else tkerr(addTk(END, line),"Error: invalid character");
                break;

            case 1:
                if(isalnum(ch) || ch == '_') pCurrent++;
                else state = 2;
                break;
            case 2:
                nCh = pCurrent - pStart;
                if(nCh==5&&!memcmp(pStart,"break",5))tk=addTk(BREAK, line);
                else if(nCh==4&&!memcmp(pStart,"char",4))tk=addTk(CHAR, line);
                else if(nCh==6&&!memcmp(pStart,"double",6))tk=addTk(DOUBLE, line);
                else if(nCh==4&&!memcmp(pStart,"else",4))tk=addTk(ELSE, line);
                else if(nCh==3&&!memcmp(pStart,"for",3))tk=addTk(FOR, line);
                else if(nCh==2&&!memcmp(pStart,"if",2))tk=addTk(IF, line);
                else if(nCh==3&&!memcmp(pStart,"int",3))tk=addTk(INT, line);
                else if(nCh==6&&!memcmp(pStart,"return",6))tk=addTk(RETURN, line);
                else if(nCh==6&&!memcmp(pStart,"struct",6))tk=addTk(STRUCT, line);
                else if(nCh==4&&!memcmp(pStart,"void",4))tk=addTk(VOID, line);
                else if(nCh==5&&!memcmp(pStart,"while",5))tk=addTk(WHILE, line);
                else{
                    tk = addTk(ID, line);
                    tk->text = malloc (sizeof(char) * (nCh + 1));
                    strcpy(tk->text, createString(pStart,pCurrent));
                }

                return tk->code;

            case 3:
                addTk(COMMA, line);
                return COMMA;
            case 4:
                addTk(SEMICOLON, line);
                return SEMICOLON;
            case 5:
                addTk(LPAR, line);
                return LPAR;
            case 6:
                addTk(RPAR, line);
                return RPAR;
            case 7:
                addTk(LBRACKET, line);
                return LBRACKET;
            case 8:
                addTk(RBRACKET, line);
                return RBRACKET;
            case 9:
                addTk(LACC, line);
                return LACC;
            case 10:
                addTk(RACC, line);
                return RACC;
            case 11:
                addTk(DOT, line);
                return DOT;
            case 12:
                if(ch == '&'){
                    pCurrent++;
                    state = 13;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;

            case 13:
                addTk(AND, line);
                return AND;
            case 14:
                if(ch == '|'){
                    pCurrent++;
                    state = 15;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 15:
                addTk(OR, line);
                return OR;
            case 16:
                if(ch == '='){
                    pCurrent++;
                    state = 18;
                } else state = 17;
                break;
            case 17:
                addTk(NOT, line);
                return NOT;
            case 18:
                addTk(NOTEQ, line);
                return NOT;
            case 19:
                if(ch == '='){
                    pCurrent++;
                    state = 21;
                } else state = 20;
                break;
            case 20:
                addTk(ASSIGN, line);
                return ASSIGN;
            case 21:
                addTk(EQUAL, line);
                return EQUAL;
            case 22:
                if(ch == '='){
                    pCurrent++;
                    state = 24;
                } else state = 23;
                break;
            case 23:
                addTk(LESS, line);
                return LESS;
            case 24:
                addTk(LESSEQ, line);
                return LESSEQ;
            case 25:
                if(ch == '='){
                    pCurrent++;
                    state = 27;
                } else state = 26;
                break;
            case 26:
                addTk(GREATER, line);
                return GREATER;
            case 27:
                addTk(GREATEREQ, line);
                return GREATEREQ;
            case 28:
                addTk(ADD, line);
                return ADD;
            case 29:
                addTk(SUB, line);
                return SUB;
            case 30:
                addTk(MUL, line);
                return MUL;
            case 31:
                if(ch == '/'){
                    pCurrent++;
                    state = 33;
                }
                else if(ch == '*'){
                    pCurrent++;
                    state = 34;
                } else state = 32;
                break;
            case 32:
                addTk(DIV, line);
                return DIV;
            case 33:
                if(ch!='\n' && ch!='\r' && ch!='\0'){
                    pCurrent++;
                } else state = 0;
                break;
            case 34:
                if(ch == '*'){
                    pCurrent++;
                    state = 35;
                }
                else if(ch == '\n'){
                    pCurrent++;
                    line++;
                } else pCurrent++;
                break;
            case 35:
                if(ch == '/'){
                    pCurrent++;
                    state = 0;
                }
                else if(ch == '*'){
                    pCurrent++;
                } else {
                    pCurrent++;
                    state = 34;
                    if(ch == '\n') line++;
                } break;
            case 36:
                if(isdigit(ch)){
                    pCurrent++;
                }
                else if(ch == '.'){
                    pCurrent++;
                    state = 43;
                }
                else if(ch == 'e' || ch == 'E'){
                    pCurrent++;
                    state = 45;
                } else state = 41;
                break;
            case 37:
                if(isdigit(ch) && ch != '8' && ch != '9'){
                    pCurrent++;
                    state = 38;
                } else if(ch == '8' || ch == '9'){
                    pCurrent++;
                    state = 42;
                } else if(ch == 'x'){
                    pCurrent++;
                    state = 39;
                } else if(ch == '.'){
                    pCurrent++;
                    state = 43;
                } else if(ch == 'e' || ch == 'E'){
                    pCurrent++;
                    state = 45;
                } else state = 41;
                break;
            case 38:
                if(isdigit(ch) && ch != '8' && ch != '9'){
                    pCurrent++;
                } else if(ch == '8' || ch == '9'){
                    pCurrent++;
                    state = 42;
                } else if(ch == '.'){
                    pCurrent++;
                    state = 43;
                } else if(ch == 'e' || ch == 'E'){
                    pCurrent++;
                    state = 45;
                } else state = 41;
                break;
            case 39:
                if(isdigit(ch) || (ch >= 'a' && ch <= 'f') ||
                   (ch >= 'A' && ch <= 'F')){
                    pCurrent++;
                    state = 40;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 40:
                if(isalnum(ch)){
                    pCurrent++;
                } else state = 41;
                break;
            case 41:
                tk = addTk(CT_INT, line);
                tk -> i = createLongInt(pStart, pCurrent);
                return CT_INT;
            case 42:
                if(isdigit(ch)){
                    pCurrent++;
                } else if(ch == '.'){
                    pCurrent++;
                    state = 43;
                } else if(ch == 'e' || ch == 'E'){
                    pCurrent++;
                    state = 45;
                }
                else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 43:
                if(isdigit(ch)){
                    pCurrent++;
                    state = 44;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 44:

                if(isdigit(ch)){
                    pCurrent++;
                } else if(ch == 'e' || ch == 'E'){
                    pCurrent++;
                    state = 45;
                } else state = 48;
                break;
            case 45:
                if(ch == '+' || ch == '-'){
                    pCurrent++;
                    state = 46;
                } else state = 46;
                break;
            case 46:
                if(isdigit(ch)){
                    pCurrent++;
                    state = 47;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 47:
                if(isdigit(ch)){
                    pCurrent++;
                } else state = 48;
                break;
            case 48:
                tk = addTk(CT_REAL, line);
                tk -> r = createDouble(pStart, pCurrent);
                return CT_REAL;
            case 49:
                if(ch == '\\'){
                    pCurrent++;
                    state = 50;
                } else if(ch != '\"'){
                    pCurrent++;
                    state = 51;
                } break;
            case 50:
                if(ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't'
                   || ch == 'v' || ch =='\'' || ch == '?' || ch == '\"' || ch == '\\' || ch == '0'){
                    pCurrent++;
                    state = 51;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 51:
                if(ch == '\''){
                    pCurrent++;
                    state = 52;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 52:
                tk = addTk(CT_CHAR, line);
                tk -> i = createLongIntChar(pStart, pCurrent);
                return CT_CHAR;
            case 53:
                if(ch == '\\'){
                    pCurrent++;
                    state = 54;
                } else if(ch != '\"'){
                    pCurrent++;
                    state = 55;
                } else if(ch == '\"'){ // else state = 55;
                    pStart++;
                    state = 56;
                } break;
            case 54:
                if(ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't'
                   || ch == 'v' || ch =='\'' || ch == '?' || ch == '\"' || ch == '\\' || ch == '0'){
                    pCurrent++;
                    state = 55;
                } else tkerr(addTk(END, line),"Error: invalid character");
                break;
            case 55:
                if(ch == '\"'){
                    pCurrent++;
                    state = 56;
                } else state = 53;
                break;
            case 56:
                tk = addTk(CT_STRING, line);
                int n = pCurrent - pStart;
                tk->text = malloc (sizeof(char) * n);
                strcpy(tk->text, createString(pStart, pCurrent));
                return CT_STRING;
            default: err("Error: state %d does not exist\n", state);
        }
    }       free(tk);
}

void freeMem()
{
    pCurrent = malloc(sizeof(char));
    free(pCurrent);
}

void showIfAttribute(struct Token *tk)
{
    if(strcmp(enumNames[tk->code], "ID") == 0 || strcmp(enumNames[tk->code], "CT_STRING") == 0)
        printf(":%s", tk->text);
    else if(strcmp(enumNames[tk->code], "CT_INT") == 0)
        printf(":%ld", tk->i);
    else if(strcmp(enumNames[tk->code], "CT_CHAR") == 0){
        printf(":%c", (char)tk->i);
    }
    else if(strcmp(enumNames[tk->code], "CT_REAL") == 0){
        printf(":%d", (int)tk->r);
        //printf(":%f", tk->r);
    }
}

void showAtoms()
{
    struct Token *p = tokens;
    while(p != NULL)
    {
        printf("%d %s", p->line, enumNames[p->code]);
        showIfAttribute(p);
        printf("\n");
        p = p->next;
    }
    printf("----------\n");
    free(p);
}

void openFileAndSetPointer(char *fileName)
{
    FILE *file = fopen(fileName, "rb");

    char fileText[30001];
    int n = fread(fileText, 1, 30000, file);
    fileText[n] = '\0';
    fclose(file);
    pCurrent = fileText;

    printf("----------\n");
    printf("%s\n", pCurrent);
    printf("----------\n");
}

Token* analyzeLex(char *fileName){

    init();
    openFileAndSetPointer(fileName);

    while(getNextToken() != END)
        ;

    showAtoms();
    freeMem();
    return tokens;
}
