#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Token.h"
#include "ErrChecker.h"
#include "SyntaxAnalyzer.h"

enum{ID, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, MUL, SUB, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, END}; // codurile AL

const char* enumNames2[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                            "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "COMMA",
                            "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC", "ADD", "MUL", "SUB",
                            "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER",
                            "GREATEREQ", "END"};
const char* enumNamesConv[] = {"ID", "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT",
                               "RETURN", "STRUCT", "VOID", "WHILE", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", ",",
                               ";", "(", ")", "[", "]", "{", "}", "+", "*", "-",
                               "DIV", "DOT", "AND", "OR", "NOT", "=", "==", "!=", "<", "<=", ">",
                               ">=", "END"};

int mainFuncFlag;
Token *consumedTk, *currentTk;
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

int typeBase(){
    if(consume(INT) || consume(CHAR) || consume(DOUBLE)){
        return 1;
    } else if(consume(STRUCT)){
        if(consume(ID)){
            return 1;
        } else tkerr(consumedTk,"Error: expected identifier(struct name) after STRUCT");
    }
    return 0;
}

void checkMain(){
    if(strcmp(consumedTk -> text, "main") == 0){
        if(mainFuncFlag == 1)
            tkerr(consumedTk, "Error: redefinition of main");
        mainFuncFlag++;
    }
}

int exprPostfix();
int exprUnary(){
    //printf("exprUnary\n");
    Token *startTk = currentTk;
    if(consume(SUB) || consume(NOT)){
        if(exprUnary())
            return 1;
        else tkerr(consumedTk,"Error: expected expression after '%s'", getName(consumedTk));
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

int declArray(){
    if(consume(LBRACKET)){
        expr();
        if(consume(RBRACKET)){
            return 1;
        } else tkerr(consumedTk,"Error: expected ']' after '%s'", getName(consumedTk));
    }
    return 0;
}

int funcArg(){
    //Token *startTk = currentTk;
    if(typeBase()){
        if(consume(ID)){
            declArray();
            return 1;
        } else tkerr(consumedTk,"Error: expected identifier(variable name) after %s", getName(consumedTk));
    }
    return 0;
}

int declVar(){
    Token *startTk = currentTk;
    if(typeBase()){
        if(consume(ID)){
            declArray();
            while(1){
                if(consume(COMMA)){
                    if(consume(ID)){
                        declArray();
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
                    } else tkerr(consumedTk,"Error: expected ';' after '%s'", getName(consumedTk));
                } else tkerr(consumedTk,"Error: expected '}' after '%s'", getName(consumedTk));
            }
        } else tkerr(consumedTk,"Error: expected identifier(struct name) after STRUCT");
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
    Token *startTk = currentTk;
    if(consume(LACC)){
        while(1){ // * branch
            if(declVar()){
                continue;
            } else if(stm()){
                continue;
            } else break;
        }
        if(consume(RACC))
            return 1;
        else tkerr(consumedTk, "Error: expected '}' after '%s'", getName(consumedTk));
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

    if(typeBase()){
        consume(MUL);
    } else if(consume(VOID)){
        isFunction = 1;
    } else return 0;

    if(consume(ID)){
        //checkMain(); // domain
        if(consume(LPAR)){
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
                if(stmCompound()){
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
    while(1){
        if(declStruct()){
            printf("unit: declared a struct\n");
        } else if(declFunc()){
            printf("unit: declared a function\n");
        } else if(declVar()){
            printf("unit: declared a variable\n");
        } else if(currentTk->code == END)
            break;
        else
        if(!typeBase())
            tkerr(currentTk, "Error: expected a type base before '%s'", getName(currentTk));
        else
            tkerr(currentTk, "Error: illegal statement - unknown");
    }

    consume(END);
    currentTk = startTk;
}

void analyzeSyntax(Token *tokens){
    mainFuncFlag = 0;
    currentTk = tokens;
    unit();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nSyntactic analysis complete: no errors found\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}