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

int mainFuncFlag = 0;
Token *consumedTk, *currentTk;
int consume(int code)
{
    if(currentTk -> code == code){
        printf("X_CONSUMED: consumed '%s'\n", enumNames2[code]);
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
    while(currentTk -> code != END){
        if(declFunc()){
            printf("info: declared a function\n");
        } else if(declVar()){
            printf("info: declared a variable\n");
        } else if(declStruct()){
            printf("info: declared a struct\n");
        } else break;
    }

    if(mainFuncFlag == 0)
        tkerr(currentTk,"Error: undefined main");

    if(!consume(END))
        tkerr(currentTk,"Error: end-of-file not found");
}

void analyzeSyntax(Token *tokens){
    currentTk = tokens;
    unit();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nSyntactic analysis complete: no errors found\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}