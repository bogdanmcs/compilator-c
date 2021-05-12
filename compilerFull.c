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
/////////////// Domain Analysis
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

char *getMemName(int mem){
    switch(mem){
        case 0:	return "MEM_GLOBAL";
            break;
        case 1: return "MEM_ARG";
            break;
        default: return "MEM_LOCAL";
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
        default: return "CLS_STRUCT";
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
        default: return "TB_VOID";
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
    //printf("Symbol(name: %s, cls: %d, mem: %d, typeBase: %d, depth: %d)\n", name, cls, s->mem, s->type.typeBase, crtDepth);
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

int mainFuncFlag;
Token *consumedTk, *currentTk;

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
    // else
    // 	printf("No vars!\n");
}


///////////////
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
int exprUnary(){
    //printf("exprUnary\n");
    Token *startTk = currentTk;
    if(consume(SUB) || consume(NOT)){
        if(exprUnary())
            return 1;
        else tkerr(consumedTk,"Error: expected expression after '%s'", getName(consumedTk));
    } else if(exprPostfix())
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
                } else tkerr(consumedTk,"Error: expected expression after '('");
            } else tkerr(consumedTk,"Error: expected ')' after '%s'", getName(consumedTk));
        } //else tkerr(consumedTk,"Error: expected type name after '('"); // ??? gcc says -no
        currentTk = startTk;
    }

    if(exprUnary())
        return 1;

    return 0;
}

int exprMul1(){
    //printf("exprMul1\n");
    Token *startTk = currentTk;
    if(consume(MUL) || consume(DIV)){
        if(exprCast()){
            if(exprMul1()){ // optional = always true
                return 1;
            }
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

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
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
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
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
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
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
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
        } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprAnd(){
    //printf("exprAnd\n");
    if(exprEq()){
        if(exprAnd1()){
            return 1;
        }
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
        }  else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        currentTk = startTk;
    }
    return 1;
}

int exprOr(){
    Token *startTk = currentTk;
    //printf("exprOr\n");
    if(exprAnd()){
        if(exprOr1()){
            return 1;
        }
    }
    currentTk = startTk;
    return 0;
}

int exprAssign(){
    //printf("exprAssign\n");
    Token *startTk = currentTk;
    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                return 1;
            } else tkerr(consumedTk, "Error: expected operand after '%s'", getName(consumedTk));
        }
    }
    // printf("trying exprOr()\n");
    currentTk = startTk;
    if(exprOr())
        return 1;

    return 0;
}

int expr(){
    //printf("expr\n");
    if(exprAssign())
        return 1;
    return 0;
}

int declArray(Type *type){
    if(consume(LBRACKET)){
        if(expr()){
            // AD
            type->nElements = 0;
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
                // Domain Analysis
                if(findSymbol(&symbols,tkName->text)){
                    tkerr(consumedTk,"Symbol redefinition: %s",tkName->text);
                } //else printf("Symbol '%s' not found, adding now\n", tkName->text);

                crtStruct = addSymbol(&symbols, tkName->text, CLS_STRUCT);
                initSymbols(&crtStruct->members);

                printf("Symbol(name: %s, cls: %s, mem: %s, depth: %d)\n", crtStruct->name,
                       getClsName(crtStruct->cls), getMemName(crtStruct->mem), crtStruct->depth);

                // Domain Analysis
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



int exprPrimary(){
    //printf("exprPrimary\n");
    Token *startTk = currentTk;
    if(consume(ID)){
        //printf(" -> id: %s\n", consumedTk->text);
        if(consume(LPAR)){
            if(expr()){
                while(1){
                    if(consume(COMMA)){
                        if(expr()){
                            continue;
                        } else tkerr(consumedTk, "Error: expected expression after ','");
                    } else break;
                }

            }
            if(!consume(RPAR))
                tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
        }
        return 1;
    } else if(consume(CT_INT)){
        //printf(" -> int: %d\n", (int)consumedTk->i);
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
            } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
        } else tkerr(consumedTk, "Error: expected expression after '('"); //?
    }
    currentTk = startTk;
    return 0;
}

int exprPostfix1(){
    //printf("exprPostfix1\n");
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfix1()){
                    //return 1;
                }
                return 1; // placed here bcs it can fail
            } else tkerr(consumedTk, "Error: expected ']' after '%s'", getName(consumedTk)); // verified
        } else tkerr(consumedTk, "Error: expected expression after '['");
    }  else
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfix1()){
                // return 1;
            }
            return 1; // placed here bcs it can fail
        } else tkerr(consumedTk, "Error: expected identifier(name) after '.'");
    }
    return 0;
}

int exprPostfix(){
    //printf("exprPostfix\n");
    if(exprPrimary()){
        if(exprPostfix1()){
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
    if(stmCompound()){
        return 1;
    } else if(consume(IF)){ // IF LPAR expr RPAR stm ( ELSE stm )?
        if(consume(LPAR)){
            if(expr()){
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
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return 1;
                    } else tkerr(consumedTk, "Error: statement missing after '('");
                } else tkerr(consumedTk, "Error: expected ')' after '%s'", getName(consumedTk));
            } else tkerr(consumedTk, "Error: expected expression after '('");
        } else tkerr(consumedTk, "Error: expected '(' after WHILE");

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
        expr();
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(consumedTk, "Error: expected ';' after '%s'", getName(consumedTk));

    } else if(expr()){ // expr SEMICOLON
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
            // printf("unit: declared a struct\n");
        } else if(declFunc()){
            //printf("unit: declared a function\n");
        } else if(declVar()){
            // printf("unit: declared a variable\n");
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
        if(aux->cls == CLS_FUNC){
            printf(", type: %s, args: ", getTypeName(syms->begin[i]->type.typeBase));
            int n2 = aux->args.end - aux->args.begin;
            for(int i2 = 0; i2 < n2; i2++){
                printf("%s ", aux->args.begin[i2]->name);
            }
            printf(")\n");
        } else if(aux->cls == CLS_STRUCT){
            printf(", members: ");
            int n2 = aux->members.end - aux->members.begin;
            for(int i2 = 0; i2 < n2; i2++){
                printf("%s ", aux->members.begin[i2]->name);
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
    openFileAndSetPointer("10.c");

    while(getNextToken() != END)
        ;

    showAtoms();

    // syntax & domain analysis
    currentTk = tokens;
    unit();
    deli();
    printf("Syntax & Domain analysis: 0 errors\n");
    deli();
    showSymbolTable();
    deli();

    //
    freeMem();
    return 0;
}