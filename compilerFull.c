#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

////////////////////////////////// Lexical analyzer

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, MUL, SUB, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, END}; // codurile AL

const char* enumNames[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                           "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "COMMA",
                           "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC", "ADD", "MUL", "SUB",
                           "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER",
                           "GREATEREQ", "END"};

typedef struct Token{
    int code, line;
    union{
        char *text;
        long int i;
        double r;
    };
    struct Token *next;
}Token;

Token *tokens, *lastToken;
char *pCurrent;
int line = 1;

void err(const char *fmt,...)
{
    va_list va;
            va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
            va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...)
{
    va_list va;
            va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
            va_end(va);
    exit(-1);
}

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

void initToken()
{
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
                if(isalnum(ch)){
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
    free(tokens);
    free(lastToken);
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

    char fileText[100];
    size_t bytesRead = 0;

    char *bufferDynamic;
    bufferDynamic = malloc (sizeof(char) * 0);

    if (file) {

        while ((bytesRead = fread(fileText, sizeof(char), sizeof(fileText), file)) > 0) {
            fileText[bytesRead] = '\0';

            bufferDynamic = realloc (bufferDynamic,
                                     sizeof(char) * (strlen(bufferDynamic) + bytesRead));
            strcat(bufferDynamic, fileText);
        }

        pCurrent = malloc (sizeof(char) * strlen(bufferDynamic));
        strcpy(pCurrent, bufferDynamic);

        printf("----------\n");
        printf("%s\n", pCurrent);
        printf("----------\n");

    } else {
        printf("Error: couldn't open file %s\n", fileName);
        exit(44);
    }

    free(bufferDynamic);
    fclose(file);
}


/////////////////////////////////////////// Syntax Analyzer
int mainFuncFlag = 0;
Token *consumedTk, *currentTk;
int consume(int code)
{
    if(currentTk -> code == code){
        printf("X: consumed '%s'\n", enumNames[code]);
        consumedTk = currentTk;
        currentTk = currentTk -> next;
        return 1;
    }
    return 0;
}

int typeBase(){
    if(consume(INT) || consume(CHAR) || consume(DOUBLE)){
        return 1;
    } else if(consume(STRUCT)){
        if(consume(ID)){
            return 1;
        } else tkerr(currentTk,"Error: 'id' missing when declaring struct: 'struct struct_name'");
    }
    return 0;
}

int checkMain(){
    if(strcmp(consumedTk -> text, "main") == 0){
        if(mainFuncFlag == 1)
            tkerr(currentTk, "Error: redefinition of main");
        mainFuncFlag++;
    }
    return 1;
}

int exprPostfix();
int exprUnary(){
    //printf("exprUnary\n");
    Token *startTk = currentTk;
    if(consume(SUB) || consume(NOT)){
        if(exprUnary())
            return 1;
        else tkerr(currentTk,"Error: exprUnary failed in exprUnary()");
    }
    else if(exprPostfix())
        return 1;

    currentTk = startTk;
    return 0;
}

int typeName();
int exprCast(){
    //printf("exprCast\n");
    Token *startTk = currentTk;
    if(consume(LPAR)){
        if(typeName()){
            if(consume(RPAR)){
                if(exprCast()){
                    return 1;
                } else tkerr(currentTk,"Error: exprCast in exprCast()");
            } else tkerr(currentTk,"Error: ')' missing in exprCast()");
        } else tkerr(currentTk,"Error: typeName in exprCast()");
    }
    else if(exprUnary()){
        return 1;
    }

    currentTk = startTk;
    return 0;
}

int exprMul1(){
    //printf("exprMul1\n");
    Token *startTk = currentTk;
    if(consume(MUL) || consume(DIV)){
        if(exprCast()){
            if(exprMul1()){
                return 1;
            }
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

// all exprXXX... should the two if's be separated by a '|' ???
int exprMul(){
    //printf("exprMul\n");
    if(exprCast()){
        if(exprMul1()){
            return 1;
        }
    }
    return 0;
}

int exprAdd1(){
    Token *startTk = currentTk;
    //printf("exprAdd1\n");
    if(consume(ADD) || consume(SUB)){
        if(exprMul()){
            if(exprAdd1()){
                return 1;
            }
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

int exprAdd(){
    //printf("exprAdd\n");
    if(exprMul()){
        if(exprAdd1()){
            return 1;
        }
    }
    return 0;
}

int exprRel1(){
    //printf("exprRel1\n");
    Token *startTk = currentTk;
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        if(exprAdd()){
            if(exprRel1()){
                return 1;
            }
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

int exprRel(){
    //printf("exprRel\n");
    if(exprAdd()){
        if(exprRel1()){
            return 1;
        }
    }
    return 0;
}

int exprEq1(){
    //printf("exprEq1\n");
    Token *startTk = currentTk;
    if(consume(EQUAL) || consume(NOTEQ)){
        if(exprRel()){
            if(exprEq1()){
                return 1;
            }
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

int exprEq(){
    //printf("exprEq\n");
    if(exprRel()){
        if(exprEq1()){
            return 1;
        }
    }
    return 0;
}

int exprAnd1(){
    //printf("exprAnd1\n");
    Token *startTk = currentTk;
    if(consume(AND)){
        if(exprEq()){
            if(exprAnd1()){
                return 1;
            }
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

int exprAnd(){
    //printf("exprAnd\n");
    if(exprEq()){
        if(exprAnd1()){
            return 1;
        } else tkerr(currentTk,"Error: exprAnd1 in exprAnd()");
    }
    return 0;
}

int exprOr1(){
    //printf("exprOr1\n");
    Token *startTk = currentTk;
    if(consume(OR)){
        if(exprAnd()){
            if(exprOr1())
                return 1;
        }
    } else return 1;
    currentTk = startTk;
    return 0;
}

// ???
int exprOr(){
    //printf("exprOr\n");
    if(exprAnd()){
        if(exprOr1()){ // separate if???
            return 1;
        } else tkerr(currentTk,"Error: exprOr1 in exprOr()");
    }
    return 0;
}

int exprAssign(){
    //printf("exprAssign\n");
    int debug, debug2;
    Token *startTk = currentTk;
    if(exprUnary()){
        if((debug = consume(ASSIGN))){
            if((debug2 = exprAssign())){
                return 1;
            }
        } //else return 0;
    }
    //printf("consume_ASSIGN failed? = %d\n", debug);
    //printf("consume_exprAssign(after ASSIGN) failed? = %d\n", debug2);
    currentTk = startTk;
    //printf("TRYING exprOr in exprAssign()\n");
    if(exprOr()){
        return 1;
    }
    currentTk = startTk;
    return 0;
}

int expr(){
    //printf("expr\n");
    if(exprAssign())
    {
        return 1;
    }
    return 0;
}

int declArray(){
    Token *startTk = currentTk;
    if(consume(LBRACKET)){
        expr(); // expr ? (optional)
        //if(!expr())
        // tkerr(currentTk, "Error: array size missing\n"); // better?
        if(consume(RBRACKET)){
            return 1;
        } else tkerr(currentTk,"Error: expected ']' before ';' token");
    }
    currentTk = startTk;
    return 0;
}

int funcArg(){
    Token *startTk = currentTk;
    if(typeBase()){
        if(consume(ID)){
            declArray();
            return 1;
        } else tkerr(currentTk,"Error: function argument needs a name after declaring its type");
    }
    currentTk = startTk;
    return 0;
}

int declVar(){
    int debug;
    Token *startTk = currentTk;
    if(typeBase()){
        if((debug = consume(ID))){

            declArray();
            while(1){
                if(consume(COMMA)){
                    if(consume(ID)){
                        declArray();
                    } else tkerr(currentTk, "Error: expected 'id' after ',' in declVar()");
                } else break;
            }
            if(consume(SEMICOLON)){
                return 1;
            } else tkerr(currentTk, "Error: missing ';' after declaration of variable");
        }
    }
    currentTk = startTk;
    return 0;
}

int declStruct(){
    Token *startTk = currentTk;
    if(consume(STRUCT)){
        if(consume(ID)){
            if(consume(LACC)){
                while(1){
                    if(declVar()){
                        continue;
                    } else
                        break;
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        return 1;
                    } else tkerr(currentTk,"Error: missing ';' in struct declaration, expected after '}'");
                } else tkerr(currentTk,"Error: '}' missing in 'struct' declaration(expected to close decl)");
            } else tkerr(currentTk,"Error: '{' missing in 'struct' declaration (expected after decl)");
        } else tkerr(currentTk,"Error: 'id' missing when declaring 'struct id'");
    }
    currentTk = startTk;
    return 0;
}

int typeName(){
    if(typeBase()){
        declArray();
        return 1;
    }
    return 0;
}



int exprPrimary(){
    //printf("exprPrimary\n");
    Token *startTk = currentTk;
    if(consume(ID)){
        printf("CONSUMED_ID: %s\n", consumedTk->text);
        if(consume(LPAR)){
            if(expr()){
                while(1){
                    if(consume(COMMA)){
                        if(expr()){
                            continue;
                        } else tkerr(currentTk, "Error: missing expression after ','");
                    } else break;
                }

            }
            if(!consume(RPAR))
                tkerr(currentTk, "Error: missing ')' in exprPrimary(ID ...)");
        }
        return 1;
    } else if(consume(CT_INT)){
        printf("CONSUMED_CT_INT: %d\n", (int)consumedTk->i);
        return 1;
    } else if(consume(CT_REAL)){
        return 1;
    } else if(consume(CT_CHAR)){
        return 1;
    } else if(consume(CT_STRING)){
        return 1;
    } else if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return 1;
            } else tkerr(currentTk, "Error: missing ')' after expr in exprPrimary(LPAR expr RPAR)");
        } else tkerr(currentTk, "Error: in exprPrimary(): expr not given (LPAR expr RPAR)");
    }
    currentTk = startTk;
    return 0;
}

// ???
int exprPostfix1(){
    //printf("exprPostfix1\n");
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfix1()){
                    //return 1;
                }
                return 1; // placed here bcs it can fail(?)
            } else tkerr(currentTk, "Error: in exprPostfix1(): missing ']' after expr");
        } else tkerr(currentTk, "Error: in exprPostfix1(): expr not found after '['");
    }  else
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfix1()){
                // return 1;
            }
            return 1; // placed here bcs it can fail(?)
        } else tkerr(currentTk, "Error: in exprPostfix1(): 'id' missing after DOT");
    }
    return 0;
}

int exprPostfix(){ // ?
    //printf("exprPostfix\n");
    if(exprPrimary()){
        if(exprPostfix1()){

        }
        return 1;
    }
    return 0;
}

int stm();
int stmCompound(){
    int debug;
    //printf("stmCompound\n");
    Token *startTk = currentTk;
    if(consume(LACC)){
        while(1){ // * branch
            if(declVar()){
                continue;
            } else if((debug = stm())){
                continue;
            } else break;
        }
        if(consume(RACC))
            return 1;
        else tkerr(currentTk, "Error: stmCompund(): '}' missing");
    }
    currentTk = startTk;
    return 0;
}

int stm(){
    //printf("stm\n");
    Token *startTk = currentTk;
    int debug;
    if(stmCompound()){
        return 1;
    } else if(consume(IF)){ // IF LPAR expr RPAR stm ( ELSE stm )?
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){ // optional branch ( ELSE stm )?
                            if(stm()){

                            } else tkerr(currentTk, "Error: stm(): missing statement after ELSE");
                        }
                        return 1;
                    }
                }
            }
        }
    } else if(consume(WHILE)){ // WHILE LPAR expr RPAR stm
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return 1;
                    }
                }
            }
        }
    } else if(consume(FOR)){ // FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
        if(consume(LPAR)){
            expr();
            if(consume(SEMICOLON)){
                expr();
                if(consume(SEMICOLON)){
                    expr();
                    if(consume(RPAR)){
                        if(stm()){
                            return 1;
                        }
                    } else tkerr(currentTk, "Error: expected ')' before for statement\n");
                } else tkerr(currentTk, "Error: expected ';' before ')' in for\n");
            } else tkerr(currentTk, "Error: expected ';' before ')' in for\n");
        } else tkerr(currentTk, "Error: expected '(' after declaration of for\n");
    } else if(consume(BREAK)){ // BREAK SEMICOLON
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(currentTk, "Error: ';' expected after BREAK");
    } else if(consume(RETURN)){ // RETURN expr? SEMICOLON
        expr();
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(currentTk, "Error: ';' expected after RETURN");
    } else if((debug = expr())){ // expr? SEMICOLON
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(currentTk, "expected ';' after expression in stm(expr+semicolon)");
    } else if(consume(SEMICOLON)){ // not(expr) SEMICOLON
        return 1;
    }

    currentTk = startTk;
    return 0;
}

int declFunc(){
    Token *startTk = currentTk;
    if(typeBase()){
        consume(MUL);
    } else if(consume(VOID)){
    } else return 0;

    if(consume(ID) && checkMain()){
        if(consume(LPAR)){
            if(funcArg()){ // optional branch
                while(1){
                    if(consume(COMMA)){
                        if(funcArg()){
                            continue;
                        } else tkerr(currentTk, "Error: declFunc(): missing function argument after ','");
                    } else break;
                }
            }

            if(consume(RPAR)){
                if(stmCompound()){
                    return 1;
                } //else if(consume(SEMICOLON)){
                //return 1;
                //}else tkerr(currentTk, "Error: statement missing after declaring function");
            } else tkerr(currentTk, "Error: missing ')' after declaring function");
            return 0;
        }
    }
    currentTk = startTk;
    return 0;
}

void unit(){
    while(currentTk->code != END){
        if(declFunc()){
            printf("info: declared a function\n");
        } else if(declVar()){
            printf("info: declared a variable\n");
        } else if(declStruct()){
            printf("info: declared a struct\n");
        } //sleep(1);
    }

    if(mainFuncFlag == 0)
        tkerr(currentTk,"Error: undefined main");

    if(!consume(END))
        tkerr(currentTk,"Error: end-of-file not found");

    printf("~~~~~~~~~~\nSyntactic analysis complete: no errors found\n~~~~~~~~~~\n");
}

int main()
{
    // lexical analyzer
    initToken();
    openFileAndSetPointer("9.c");

    while(getNextToken() != END)
        ;

    showAtoms();

    //syntax analyzer
    currentTk = tokens;
    unit();

    // free memory and exit program
    freeMem();
    return 0;
}