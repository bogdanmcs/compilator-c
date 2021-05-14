#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

////////////////////////////////// Lexical analysis

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, MUL, SUB, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, END}; // codurile AL

const char* enumNames[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                           "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "COMMA",
                           "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC", "ADD", "MUL", "SUB",
                           "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER",
                           "GREATEREQ", "END"};
const char* enumNamesConv[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                               "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", ",",
                               ";", "(", ")", "[", "]", "{", "}", "+", "*", "-",
                               "/", ".", "&&", "||", "!", "=", "==", "!=", "<", "<=", ">",
                               ">=", "END"};


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
        //printf(":%d", (int)tk->r);
        printf(":%f", tk->r);
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
    FILE *file;
    if((file = fopen(fileName, "rb")) == NULL){
        printf("Error: cannot open file '%s' / file doesn't exist / invalid path\n",
               fileName);
        exit(13);
    }

    char fileText[30001];
    int n = fread(fileText, 1, 30000, file);
    fileText[n] = '\0';
    fclose(file);
    pCurrent = fileText;

    printf("----------\n");
    printf("%s\n", pCurrent);
    printf("----------\n");
}


/////////////////////////////////////////// Syntax Analysis
////////////////////////////// Domain Analysis
///////////////////////// Type Analysis
enum {TB_INT,TB_DOUBLE,TB_CHAR,TB_STRUCT,TB_VOID};

struct _Symbol;
typedef struct _Symbol Symbol;

typedef struct{
    Symbol **begin;
    Symbol **end;
    Symbol **after;
} Symbols;

typedef struct{
    int typeBase;
    Symbol *s;
    int nElements;
} Type;

enum {CLS_VAR,CLS_FUNC,CLS_EXTFUNC,CLS_STRUCT};
enum {MEM_GLOBAL,MEM_ARG,MEM_LOCAL};

typedef struct _Symbol{
    const char *name;
    int cls;
    int mem;
    Type type;
    int depth;
    union{
        Symbols args;
        Symbols members;
    };
} Symbol;

Symbols symbols;

void initSymbols(Symbols *symbols){
    symbols->begin=NULL;
    symbols->end=NULL;
    symbols->after=NULL;
}

int crtDepth = 0;
Symbol *crtFunc = NULL;
Symbol *crtStruct = NULL;

Token *consumedTk, *currentTk;

Type createType(int typeBase, int nElements)
{
    Type t;
    t.typeBase=typeBase;
    t.nElements=nElements;
    return t;
}

typedef union{
    long int i;
    double d;
    const char *str;
} CtVal;

typedef struct{
    Type type;
    int isLVal;
    int isCtVal;
    CtVal ctVal;
} RetVal;

void cast(Type *dst,Type *src)
{
    if(src->nElements >- 1){
        if(dst->nElements >- 1){
            if(src->typeBase!=dst->typeBase)
                tkerr(currentTk,"an array cannot be converted to an array of another type");
        } else {
            tkerr(currentTk,"an array cannot be converted to a non-array");
        }
    } else {
        if(dst->nElements >- 1){
            tkerr(currentTk,"a non-array cannot be converted to an array");
        }
    }

    switch(src->typeBase){
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:

            switch(dst->typeBase){
                case TB_CHAR:
                case TB_INT:
                case TB_DOUBLE:
                    return;
            }

        case TB_STRUCT:
            if(dst->typeBase==TB_STRUCT){
                if(src->s!=dst->s)
                    tkerr(currentTk,"a structure cannot be converted to another one");
                return;
            }
    }
    tkerr(currentTk,"incompatible types");
}

Type getArithType(Type *s1,Type *s2){
    Type convType;
    convType.nElements = -1;

    if(s1->typeBase == s2->typeBase){
        convType.typeBase = s1->typeBase;
    } else if((s1->typeBase == TB_INT && s2->typeBase == TB_CHAR) || (s1->typeBase == TB_CHAR && s2->typeBase == TB_INT)){
        convType.typeBase = TB_INT;
    } else if((s1->typeBase == TB_INT && s2->typeBase == TB_DOUBLE) || (s1->typeBase == TB_DOUBLE && s2->typeBase == TB_INT)){
        convType.typeBase = TB_DOUBLE;
    } else if((s1->typeBase == TB_CHAR && s2->typeBase == TB_DOUBLE)|| (s1->typeBase == TB_DOUBLE && s2->typeBase == TB_CHAR)){
        convType.typeBase = TB_DOUBLE;
    } else {
        tkerr(currentTk, "unknown type");
    }

    return convType;
}

char *getMemName(int mem){
    switch(mem){
        case 0:	return "MEM_GLOBAL";
            break;
        case 1: return "MEM_ARG";
            break;
        case 2: return "MEM_LOCAL";
            break;
        default: return "NO_MEM";
    }
}

char *getClsName(int cls){
    switch(cls){
        case 0:	return "CLS_VAR";
            break;
        case 1: return "CLS_FUNC";
            break;
        case 2: return "CLS_EXTFUNC";
            break;
        case 3: return "CLS_STRUCT";
            break;
        default: return "NO_CLS";
    }
}

char *getTypeName(int type){
    switch(type){
        case 0:	return "TB_INT";
            break;
        case 1: return "TB_DOUBLE";
            break;
        case 2: return "TB_CHAR";
            break;
        case 3:	return "TB_STRUCT";
            break;
        case 4: return "TB_VOID";
            break;
        default: return "NO_TB";
    }
}


Symbol *addSymbol(Symbols *symbols, const char *name, int cls){
    Symbol *s;
    if(symbols->end == symbols->after){
        int count =  symbols->after-symbols->begin;
        int n = count * 2;

        if(n == 0)
            n = 1;

        symbols->begin = (Symbol**) realloc (symbols->begin, n * sizeof(Symbol*));

        if(symbols->begin == NULL)
            err("not enough memory");

        symbols->end = symbols->begin + count;
        symbols->after = symbols->begin + n;
    }

    SAFEALLOC (s,Symbol) * symbols->end++ = s;
    s->name = name;
    s->cls = cls;
    s->depth = crtDepth;
    return s;
}

Symbol *findSymbol(Symbols *symbols, const char *name){
    int i, n = symbols->end - symbols->begin;
    for(i = n - 1; i >= 0; i--){
        if(strcmp(symbols->begin[i]->name, name) == 0)
            return symbols->begin[i];
    }
    return NULL;
}

Symbol *findSymbolInFunctionArgs(Symbols *symbols, const char *name){
    int i, n = symbols->end - symbols->begin;
    for(i = n - 1; i >= 0 && symbols->begin[i]->cls != CLS_FUNC; i--){
        if(strcmp(symbols->begin[i]->name, name) == 0)
            return symbols->begin[i];
    }
    return NULL;
}

void addVar(Token *tkName,Type *t){
    Symbol *s;

    if(crtStruct){
        if(findSymbol(&crtStruct->members, tkName->text))
            tkerr(currentTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
    }
    else if(crtFunc){
        s = findSymbol(&symbols, tkName->text);
        if(s&&s->depth == crtDepth)
            tkerr(currentTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL;
    } else{
        if(findSymbol(&symbols, tkName->text))
            tkerr(currentTk, "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }
    s->type = *t;

    if(crtStruct != NULL){
        printf("Symbol_struct_var(name: %s, cls: %s, mem: %s, type: %s, depth: %d)\n", s->name,
               getClsName(s->cls), getMemName(s->mem), getTypeName(s->type.typeBase), s->depth);
    } else {
        printf("Symbol(name: %s, cls: %s, mem: %s, type: %s, depth: %d)\n", s->name,
               getClsName(s->cls), getMemName(s->mem), getTypeName(s->type.typeBase), s->depth);
    }
}

void deleteSymbolsAfter(Symbols *symbols, Symbol *crtDel){
    //printf("Deleting after: %s\n\n", crtDel->name);

    if(crtDel != symbols->end[-1]){
        Symbol **i;

        for(i = symbols->begin; i != symbols->end; i++){
            //printf("%s ", (*i)->name);
            if((*i) == crtDel){
                i++;
                break;
            }
        } //printf("\n");

        (*i) = NULL;
        symbols->end = i;
    }
    // else `no vars to delete`
}

Symbol *addExtFunc(const char *name,Type type){
    Symbol *s = addSymbol(&symbols, name, CLS_EXTFUNC);
    s->type = type;
    initSymbols(&s->args);
    return s;
}

Symbol *addFuncArg(Symbol *func,const char *name,Type type){
    Symbol *a = addSymbol(&func->args, name, CLS_VAR);
    a->type = type;
    return a;
}


//////////////////////////////
int consume(int code)
{
    if(currentTk -> code == code){
        //printf("_consumed: '%s'\n", enumNames[code]);
        consumedTk = currentTk;
        currentTk = currentTk -> next;
        return 1;
    }
    return 0;
}

char* getName(Token *token)
{
    char charConv[50];
    if(token->code == ID || token->code == CT_STRING){
        strcpy(charConv, token->text);
    } else if(token->code == CT_INT || token->code == CT_CHAR){
        snprintf(charConv, 49, "%ld", token->i);
    } else if(token->code == CT_REAL){
        snprintf(charConv, 49, "%f", token->r);
    } else {
        strcpy(charConv, enumNamesConv[token->code]);
    }

    char *charConvF = malloc (sizeof(char) * strlen(charConv));
    strcpy(charConvF, charConv);
    return charConvF;
}

int typeBase(Type *type){
    Token *tkName;

    if(consume(INT)){
        type->typeBase = TB_INT;
        return 1;
    } else if(consume(CHAR)){
        type->typeBase = TB_CHAR;
        return 1;
    } else if(consume(DOUBLE)){
        type->typeBase = TB_DOUBLE;
        return 1;
    } else if(consume(STRUCT)){
        if(consume(ID)){
            tkName = consumedTk;

            Symbol *s = findSymbol(&symbols, tkName->text);
            if(s == NULL){
                tkerr(currentTk,"Undefined symbol: %s", tkName->text);
            }

            if(s->cls != CLS_STRUCT){
                tkerr(currentTk,"%s is not a struct", tkName->text);
            }
            type->typeBase = TB_STRUCT;
            type->s = s;

            return 1;
        } else tkerr(consumedTk,"Error: expected identifier(struct name) after STRUCT");
    }
    return 0;
}

int exprPostfix();
int exprUnary(RetVal *rv){
    //printf("exprUnary\n");
    Token *startTk = currentTk;
    Token *tkop;

    if(consume(SUB) || consume(NOT)){
        tkop = consumedTk;
        if(exprUnary(rv)){
            if(tkop->code == SUB){
                if(rv->type.nElements>=0) tkerr(currentTk,"unary '-' cannot be applied to an array");
                if(rv->type.typeBase == TB_STRUCT)
                    tkerr(currentTk,"unary '-' cannot be applied to a struct");
            } else {  // NOT
                if(rv->type.typeBase == TB_STRUCT) tkerr(currentTk,"'!' cannot be applied to a struct");
                rv->type = createType(TB_INT, -1);
            }
            rv->isCtVal = rv->isLVal = 0;

            return 1;
        }
        else tkerr(consumedTk,"Error: expected expression after '%s'", getName(consumedTk));
    } else if(exprPostfix(rv))
        return 1;

    currentTk = startTk;
    return 0;
}

int typeName();
int exprCast(RetVal *rv){
    //printf("exprCast\n");
    Token *startTk = currentTk;
    RetVal rve;
    Type *type = malloc (sizeof(Type));

    if(consume(LPAR)){
        if(typeName(type)){
            if(consume(RPAR)){
                if(exprCast(&rve)){
                    cast(type ,&rve.type);
                    rv->type = *type; // *type ?
                    rv->isCtVal = rv -> isLVal = 0;

                    return 1;
                } else tkerr(consumedTk,"Error: expected expression after '('");
            } else tkerr(consumedTk,"Error: expected ')' after '%s'", getName(consumedTk));
        } //else tkerr(consumedTk,"Error: expected type name after '('"); // ??? gcc says -no
        currentTk = startTk;
    }

    if(exprUnary(rv))
        return 1;

    return 0;
}

int exprMul1(RetVal *rv){
    //printf("exprMul1\n");
    Token *startTk = currentTk;
    RetVal rve;
    Token *tkop;

    if(consume(MUL) || consume(DIV)){
        tkop = consumedTk;
        if(exprCast(&rve)){
            if(rv->type.nElements > -1 || rve.type.nElements > -1)
                tkerr(currentTk,"an array cannot be multiplied or divided");
            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
                tkerr(currentTk,"a structure cannot be multiplied or divided");
            if(rv->type.typeBase == TB_VOID || rve.type.typeBase == TB_VOID)
                tkerr(currentTk,"a VOID function cannot be multiplied or divided");

            rv->type = getArithType(&rv->type, &rve.type);
            rv->isCtVal = rv->isLVal = 0;

            if(exprMul1(rv)){ // optional
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprMul(RetVal *rv){
    //printf("exprMul\n");
    if(exprCast(rv)){
        if(exprMul1(rv)){
            return 1;
        }
    }
    return 0;
}

int exprAdd1(RetVal *rv){
    Token *startTk = currentTk;
    //printf("exprAdd1\n");
    RetVal rve;
    Token *tkop;

    if(consume(ADD) || consume(SUB)){
        tkop = consumedTk;
        if(exprMul(&rve)){
            if(rv->type.nElements > -1||rve.type.nElements >- 1)
                tkerr(currentTk,"an array cannot be added or subtracted");
            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
                tkerr(currentTk,"a structure cannot be added or subtracted");
            if(rv->type.typeBase == TB_VOID || rve.type.typeBase == TB_VOID)
                tkerr(currentTk,"a VOID function cannot be added or subtracted");

            rv->type = getArithType(&rv->type,&rve.type);
            rv->isCtVal = rv->isLVal = 0;

            if(exprAdd1(rv)){
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprAdd(RetVal *rv){
    //printf("exprAdd\n");
    if(exprMul(rv)){
        if(exprAdd1(rv)){
            return 1;
        }
    }
    return 0;
}

int exprRel1(RetVal *rv){
    //printf("exprRel1\n");
    Token *startTk = currentTk;
    RetVal rve;
    Token *tkop;

    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        tkop = consumedTk;
        if(exprAdd(&rve)){
            if(rv->type.nElements > -1 || rve.type.nElements > -1)
            {tkerr(currentTk,"an array cannot be compared");}
            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {tkerr(currentTk,"a structure cannot be compared");}
            rv->type = createType(TB_INT,-1);
            rv->isCtVal = rv->isLVal = 0;

            if(exprRel1(rv)){
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprRel(RetVal *rv){
    //printf("exprRel\n");
    if(exprAdd(rv)){
        if(exprRel1(rv)){
            return 1;
        }
    }
    return 0;
}

int exprEq1(RetVal *rv){
    //printf("exprEq1\n");
    Token *startTk = currentTk;
    RetVal rve;
    Token *tkop;

    if(consume(EQUAL) || consume(NOTEQ)){
        tkop = consumedTk;
        if(exprRel(&rve)){
            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT){
                tkerr(currentTk,"a structure cannot be compared");
            }

            rv->type = createType(TB_INT,-1);
            rv->isCtVal = rv->isLVal = 0;

            if(exprEq1(rv)){
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprEq(RetVal *rv){
    //printf("exprEq\n");
    if(exprRel(rv)){
        if(exprEq1(rv)){
            return 1;
        }
    }
    return 0;
}

int exprAnd1(RetVal *rv){
    //printf("exprAnd1\n");
    Token *startTk = currentTk;
    RetVal rve;

    if(consume(AND)){
        if(exprEq(&rve)){
            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT){
                tkerr(currentTk,"a structure cannot be logically tested");
            }

            rv->type = createType(TB_INT,-1);
            rv->isCtVal = rv->isLVal = 0;

            if(exprAnd1(rv)){
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprAnd(RetVal *rv){
    //printf("exprAnd\n");
    if(exprEq(rv)){
        if(exprAnd1(rv)){
            return 1;
        }
    }
    return 0;
}

int exprOr1(RetVal *rv){
    //printf("exprOr1\n");
    Token *startTk = currentTk;
    RetVal rve;

    if(consume(OR)){
        if(exprAnd(&rve)){

            if(rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT){
                tkerr(currentTk,"a structure cannot be logically tested");
            }

            rv->type = createType(TB_INT,-1);
            rv->isCtVal = rv->isLVal = 0;

            if(exprOr1(rv))
                return 1;
        }  else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprOr(RetVal *rv){
    Token *startTk = currentTk;
    //printf("exprOr\n");
    if(exprAnd(rv)){
        if(exprOr1(rv)){
            return 1;
        }
    }
    currentTk = startTk;
    return 0;
}

int exprAssign(RetVal *rv){
    //printf("exprAssign\n");
    RetVal rve;

    Token *startTk = currentTk;
    if(exprUnary(rv)){
        if(consume(ASSIGN)){
            if(exprAssign(&rve)){
                // TD
                if(!rv->isLVal) tkerr(currentTk,"cannot assign to a non-lval");
                if(rv->type.nElements > -1 || rve.type.nElements > -1) tkerr(currentTk,"the arrays cannot be assigned");
                cast(&rv->type, &rve.type);
                rv->isCtVal = rv->isLVal = 0;

                return 1;
            } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        }
    }

    currentTk = startTk;
    if(exprOr(rv))
        return 1;

    return 0;
}

int expr(RetVal *rv){
    //printf("expr\n");
    if(exprAssign(rv))
        return 1;
    return 0;
}

int declArray(Type *type){
    if(consume(LBRACKET)){
        RetVal rv;
        if(expr(&rv)){
            type->nElements = 0;

            if(!rv.isCtVal) tkerr(currentTk,"the array size is not a constant");
            if(rv.type.typeBase != TB_INT) tkerr(currentTk,"the array size is not an integer");
            type->nElements = rv.ctVal.i;

        } else {
            type->nElements=0;
        }

        if(consume(RBRACKET)){
            return 1;
        } else tkerr(consumedTk,"Error: expected ']' after '%s'", getName(consumedTk));
    }
    return 0;
}

int funcArg(){
    Type *type = malloc(sizeof(Type));
    Token *tkName;

    if(typeBase(type)){
        if(consume(ID)){
            tkName = consumedTk;

            // verify if it's the only arg with this name
            if(findSymbolInFunctionArgs(&symbols, tkName->text))
                tkerr(consumedTk,"Symbol redefinition: %s",tkName->text);


            if(!declArray(type)){
                type->nElements = -1;
            }

            Symbol *s = addSymbol(&symbols, tkName->text, CLS_VAR);
            s->mem = MEM_ARG;
            s->type = *type;
            s = addSymbol(&crtFunc->args, tkName->text, CLS_VAR);
            s->mem = MEM_ARG;
            s->type = *type;

            printf("Symbol(name: %s, cls: %s, mem: %s, type: %s, depth: %d)\n", s->name,
                   getClsName(s->cls), getMemName(s->mem), getTypeName(s->type.typeBase), s->depth);

            return 1;
        } else tkerr(consumedTk,"Error: expected identifier(variable name) after %s", getName(consumedTk));
    }
    return 0;
}

int declVar(){
    Token *startTk = currentTk;
    Type *type = malloc (sizeof(Type));
    Token *tkName;

    if(typeBase(type)){
        if(consume(ID)){
            tkName = consumedTk;
            if(!declArray(type)){
                type->nElements = -1;
            }

            addVar(tkName, type);

            while(1){
                if(consume(COMMA)){
                    if(consume(ID)){
                        tkName = consumedTk;
                        if(!declArray(type)){
                            type->nElements = -1;
                        }

                        addVar(tkName, type);

                    } else tkerr(consumedTk, "Error: expected identifier(variable name) after ','");
                } else break;
            }
            if(consume(SEMICOLON)){
                return 1;
            } else tkerr(consumedTk, "Error: expected ';' after '%s'", getName(consumedTk));
        } else tkerr(consumedTk, "Error: expected identifier(variable name) after '%s'", getName(consumedTk));
    }
    currentTk = startTk;
    return 0;
}

int declStruct(){
    Token *startTk = currentTk;
    Token *tkName;

    if(consume(STRUCT)){
        if(consume(ID)){
            tkName = consumedTk;
            if(consume(LACC)){

                if(findSymbol(&symbols,tkName->text)){
                    tkerr(consumedTk,"Symbol redefinition: %s",tkName->text);
                } //else printf("Symbol '%s' not found, adding now\n", tkName->text);

                crtStruct = addSymbol(&symbols, tkName->text, CLS_STRUCT);
                initSymbols(&crtStruct->members);

                crtStruct->type.typeBase = -1; // ?
                crtStruct->type.nElements = -1; // ?

                printf("Symbol(name: %s, cls: %s, mem: %s, depth: %d)\n", crtStruct->name,
                       getClsName(crtStruct->cls), getMemName(crtStruct->mem), crtStruct->depth);


                while(1){
                    if(declVar()){
                        continue;
                    } else
                        break;
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        crtStruct = NULL;
                        return 1;
                    } else tkerr(consumedTk,"Error: expected ';' after '%s'", getName(consumedTk));
                } else tkerr(currentTk,"Error: expected '}' before '%s'", getName(consumedTk));
            }
        } else tkerr(consumedTk,"Error: expected identifier(struct name) after STRUCT");
    }
    currentTk = startTk;
    return 0;
}

int typeName(Type *type){
    if(typeBase(type)){
        if(!declArray(type)){
            type->nElements = -1;
        }
        return 1;
    }
    return 0;
}



int exprPrimary(RetVal *rv){
    //printf("exprPrimary\n");
    Token *startTk = currentTk;
    Token *tkName;
    RetVal arg;

    if(consume(ID)){
        tkName = consumedTk;

        Symbol *s = findSymbol(&symbols, tkName->text);
        if(!s) tkerr(currentTk,"undefined symbol %s",tkName->text);
        rv->type = s->type;
        rv->isCtVal = 0;
        rv->isLVal = 1;

        //printf(" -> id: %s\n", consumedTk->text);
        if(consume(LPAR)){
            Symbol **crtDefArg = s->args.begin;
            if(s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
                tkerr(currentTk,"call of the non-function %s",tkName->text);

            if(expr(&arg)){
                if(crtDefArg==s->args.end) tkerr(currentTk,"too many arguments in call");
                cast(&(*crtDefArg)->type, &arg.type);
                crtDefArg++;

                while(1){
                    if(consume(COMMA)){
                        if(expr(&arg)){
                            if(crtDefArg == s->args.end) tkerr(currentTk,"too many arguments in call");
                            cast(&(*crtDefArg)->type, &arg.type);
                            crtDefArg++;

                            continue;
                        } else tkerr(consumedTk, "Error: expected expression after ','");
                    } else break;
                }

            }
            if(!consume(RPAR)){
                tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
            } else {
                if(crtDefArg != s->args.end) tkerr(currentTk,"too few arguments in call");
                rv->type = s->type;
                rv->isCtVal = rv->isLVal = 0;
            }
        } else {
            if(s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
                tkerr(currentTk,"missing call for function %s", tkName->text);
        }
        //

        return 1;
    } else if(consume(CT_INT)){
        Token *tki = consumedTk;
        rv->type = createType(TB_INT, -1);
        rv->ctVal.i = tki->i;
        rv->isCtVal = 1;
        rv->isLVal = 0;

        return 1;
    } else if(consume(CT_REAL)){
        Token *tkr = consumedTk;
        rv->type = createType(TB_DOUBLE, -1);
        rv->ctVal.d = tkr->r;
        rv->isCtVal = 1;
        rv->isLVal = 0;

        return 1;
    } else if(consume(CT_CHAR)){
        Token *tkc = consumedTk;
        rv->type = createType(TB_CHAR, -1);
        rv->ctVal.i = tkc->i;
        rv->isCtVal = 1;
        rv->isLVal = 0;

        return 1;
    } else if(consume(CT_STRING)){
        Token *tks = consumedTk;
        rv->type = createType(TB_CHAR, 0);
        rv->ctVal.str = tks->text;
        rv->isCtVal = 1;
        rv->isLVal = 0;

        return 1;
    } else if(consume(LPAR)){
        if(expr(rv)){
            if(consume(RPAR)){
                return 1;
            } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
        } else tkerr(consumedTk, "Error: expected expression after '('"); //?
    }
    currentTk = startTk;
    return 0;
}

// TD ??
int exprPostfix1(RetVal *rv){
    //printf("exprPostfix1\n");
    RetVal rve;
    Token *tkName;

    if(consume(LBRACKET)){
        if(expr(&rve)){
            if(rv->type.nElements < 0) tkerr(currentTk,"only an array can be indexed");

            Type typeInt = createType(TB_INT,-1);
            cast(&typeInt, &rve.type);
            rv->type = rv->type;
            rv->type.nElements = -1;
            rv->isLVal = 1;
            rv->isCtVal = 0;

            if(consume(RBRACKET)){
                if(exprPostfix1(rv)){
                    //return 1;
                }
                return 1; // placed here bcs it can fail
            } else tkerr(consumedTk, "Error: expected ']' after '%s'", getName(consumedTk)); // verified
        } else tkerr(consumedTk, "Error: expected expression after '['");
    }  else
    if(consume(DOT)){
        if(consume(ID)){
            tkName = consumedTk;

            Symbol *sStruct = rv->type.s;
            Symbol *sMember = findSymbol(&sStruct->members, tkName->text);
            if(!sMember)
                tkerr(currentTk,"struct %s does not have a member %s", sStruct->name, tkName->text);
            rv->type = sMember->type;
            rv->isLVal = 1;
            rv->isCtVal = 0;

            if(exprPostfix1(rv)){
                // return 1;
            }
            return 1; // placed here bcs it can fail
        } else tkerr(consumedTk, "Error: expected identifier(name) after '.'");
    }
    return 0;
}

int exprPostfix(RetVal *rv){
    //printf("exprPostfix\n");
    if(exprPrimary(rv)){
        if(exprPostfix1(rv)){
            // opt
        }
        return 1;
    }
    return 0;
}

int stm();
int stmCompound(){
    //printf("stmCompound\n");
    Symbol *start = symbols.end[-1];

    Token *startTk = currentTk;
    if(consume(LACC)){
        crtDepth++;
        while(1){ // * branch
            if(declVar()){
                continue;
            } else if(stm()){
                continue;
            } else break;
        }
        if(consume(RACC)){
            crtDepth--;
            deleteSymbolsAfter(&symbols, start);

            return 1;
        }
        else tkerr(currentTk, "Error: expected '}' before '%s'", getName(currentTk));
    }
    currentTk = startTk;
    return 0;
}

int stm(){
    //printf("stm\n");
    Token *startTk = currentTk;
    RetVal rv, rv1, rv2, rv3;

    if(stmCompound()){
        return 1;
    } else if(consume(IF)){ // IF LPAR expr RPAR stm ( ELSE stm )?
        if(consume(LPAR)){
            if(expr(&rv)){
                if(rv.type.typeBase == TB_STRUCT)
                    tkerr(currentTk,"a structure cannot be logically tested");
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){ // optional branch ( ELSE stm )?
                            if(stm()){

                            } else tkerr(consumedTk, "Error: missing statement after ELSE");
                        }
                        return 1;
                    } else tkerr(consumedTk, "Error: missing statement after '('");
                } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
            } else tkerr(consumedTk, "Error: expected expression after '('");
        } else tkerr(consumedTk, "Error: expected '(' after IF");

    } else if(consume(WHILE)){ // WHILE LPAR expr RPAR stm
        if(consume(LPAR)){
            if(expr(&rv)){
                if(rv.type.typeBase == TB_STRUCT)
                    tkerr(currentTk,"a structure cannot be logically tested");
                if(consume(RPAR)){
                    if(stm()){
                        return 1;
                    } else tkerr(consumedTk, "Error: statement missing after '('");
                } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
            } else tkerr(consumedTk, "Error: expected expression after '('");
        } else tkerr(consumedTk, "Error: expected '(' after WHILE");

    } else if(consume(FOR)){ // FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
        if(consume(LPAR)){
            if(expr(&rv1)){

            }
            if(consume(SEMICOLON)){
                if(expr(&rv2)){
                    if(rv2.type.typeBase == TB_STRUCT)
                        tkerr(currentTk,"a structure cannot be logically tested");
                }
                if(consume(SEMICOLON)){
                    if(expr(&rv3)){

                    }
                    if(consume(RPAR)){
                        if(stm()){
                            return 1;
                        } else tkerr(consumedTk, "Error: statement missing after ')'");
                    } else tkerr(consumedTk, "Error: expected ')' after '%s'\n", getName(consumedTk));
                } else tkerr(consumedTk, "Error: expected ';' after '%s' in FOR\n", getName(consumedTk));
            } else tkerr(consumedTk, "Error: expected ';' after '%s' in FOR\n", getName(consumedTk));
        } else tkerr(consumedTk, "Error: expected '(' after FOR\n");

    } else if(consume(BREAK)){ // BREAK SEMICOLON
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(consumedTk, "Error: expected ';'' after BREAK");

    } else if(consume(RETURN)){ // RETURN expr? SEMICOLON
        if(expr(&rv)){
            if(crtFunc->type.typeBase == TB_VOID){
                tkerr(currentTk,"a void function cannot return a value");
            }
            cast(&crtFunc->type, &rv.type);
        }
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(consumedTk, "Error: expected ';' after '%s'", getName(consumedTk));

    } else if(expr(&rv)){ // expr SEMICOLON
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(consumedTk, "expected ';' after '%s'", getName(consumedTk));

    } else if(consume(SEMICOLON)){ //  SEMICOLON
        return 1;
    }

    currentTk = startTk;
    return 0;
}

int declFunc(){
    Token *startTk = currentTk;
    int isFunction = 0;
    int isStruct = 0;

    if(currentTk->code == STRUCT)
        isStruct = 1;

    Type *type = malloc (sizeof(Type));
    Token *tkName;

    if(typeBase(type)){
        if(consume(MUL)){
            type->nElements = 0;
        } else {
            type->nElements = -1;
        }
    } else if(consume(VOID)){
        type->typeBase = TB_VOID;
        type->nElements = -1;
        isFunction = 1;
    } else return 0;

    if(consume(ID)){
        tkName = consumedTk;

        if(consume(LPAR)){
            if(findSymbol(&symbols, tkName->text)){
                tkerr(currentTk,"Symbol redefinition: %s", tkName->text);
            } //else printf("Symbol '%s' not found, adding now\n", tkName->text);
            crtFunc = addSymbol(&symbols, tkName->text, CLS_FUNC);
            initSymbols(&crtFunc->args);
            crtFunc->type = *type;
            printf("Symbol(name: %s, cls: %s, mem: %s, type: %s, depth: %d)\n", crtFunc->name,
                   getClsName(crtFunc->cls), getMemName(crtFunc->mem), getTypeName(crtFunc->type.typeBase), crtFunc->depth);
            crtDepth++;

            if(funcArg()){ // optional branch
                while(1){
                    if(consume(COMMA)){
                        if(funcArg()){
                            continue;
                        } else tkerr(consumedTk, "Error: missing function argument type after ','");
                    } else break;
                }
            }

            if(consume(RPAR)){
                crtDepth--;
                if(stmCompound()){
                    deleteSymbolsAfter(&symbols, crtFunc);
                    crtFunc = NULL;

                    return 1;
                } else tkerr(consumedTk, "Error: expected '{' after '%s'", getName(consumedTk));
            } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
            return 0;
        }
        if(isFunction) tkerr(consumedTk, "Error: expected '(' after '%s'", getName(consumedTk));

    } else {
        if(!isStruct && currentTk->code == LPAR)
            tkerr(consumedTk, "Error: expected function name after '%s'", getName(consumedTk));
    }
    currentTk = startTk;
    return 0;
}

void unit(){
    Token *startTk = currentTk;
    Type *type = malloc(sizeof(Type));
    while(1){
        if(declStruct()){
        } else if(declFunc()){
        } else if(declVar()){
        } else if(currentTk->code == END)
            break;
        else
        if(!typeBase(type))
            tkerr(currentTk, "Error: expected a type base before '%s'", getName(currentTk));
        else
            tkerr(currentTk, "Error: illegal statement - unknown");
    }

    consume(END);
    currentTk = startTk;
}

void showSymbolTable(){
    Symbols *syms = &symbols;
    int n = syms->end - syms->begin;

    printf("Show TS(%d):\n", n);

    for(int i = 0; i < n; i++){
        printf("Symbol(name: %s, cls: %s, mem: %s",
               syms->begin[i]->name, getClsName(syms->begin[i]->cls),
               getMemName(syms->begin[i]->mem));

        Symbol *aux = syms->begin[i];

        if(aux->cls == CLS_FUNC || aux->cls == CLS_EXTFUNC){
            printf(", type: %s, args: ", getTypeName(aux->type.typeBase));
            int n2 = aux->args.end - aux->args.begin;
            for(int i2 = 0; i2 < n2; i2++){
                printf("%s(typeBase: %s, nElements: %d) ", aux->args.begin[i2]->name, getTypeName(aux->args.begin[i2]->type.typeBase), aux->args.begin[i2]->type.nElements);
            }
            printf(")\n");

        } else if(aux->cls == CLS_STRUCT){
            printf(", members: ");
            int n2 = aux->members.end - aux->members.begin;
            for(int i2 = 0; i2 < n2; i2++){
                printf("%s(typeBase: %s, nElements: %d) ", aux->members.begin[i2]->name, getTypeName(aux->args.begin[i2]->type.typeBase), aux->args.begin[i2]->type.nElements);
            }
            printf(")\n");

        } else {
            printf(", type: %s)\n", getTypeName(syms->begin[i]->type.typeBase));
        }
    }
}

void deli(){
    printf("----------\n");
}

int main()
{
    // lexical analysis
    initToken();
    openFileAndSetPointer("9.c");

    while(getNextToken() != END)
        ;

    showAtoms();

    // syntax & domain analysis
    currentTk = tokens;

    Symbol *s;
    s = addExtFunc("put_s",createType(TB_VOID,-1));
    addFuncArg(s,"s",createType(TB_CHAR,0));
    s = addExtFunc("get_s",createType(TB_VOID,-1));
    addFuncArg(s,"s",createType(TB_CHAR,0));
    s = addExtFunc("put_i",createType(TB_VOID,-1));
    addFuncArg(s,"i",createType(TB_INT,-1));
    s = addExtFunc("get_i",createType(TB_INT,-1));
    s = addExtFunc("pud_d",createType(TB_VOID,-1));
    addFuncArg(s,"d",createType(TB_DOUBLE, -1));
    s = addExtFunc("get_d",createType(TB_DOUBLE,-1));
    s = addExtFunc("put_c",createType(TB_VOID,-1));
    addFuncArg(s,"c",createType(TB_CHAR, -1));
    s = addExtFunc("get_c",createType(TB_CHAR,-1));
    s = addExtFunc("seconds",createType(TB_DOUBLE,-1));

    unit();
    deli();
    printf("Syntax & Domain analysis: 0 errors\n");
    deli();
    showSymbolTable();
    deli();

    // type analysis


    //
    freeMem();
    return 0;
}