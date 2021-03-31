#ifndef TOKEN_H
#define TOKEN_H

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

typedef struct Token{
    int code, line;
    union{
        char *text;
        long int i;
        double r;
    };
    struct Token *next;
}Token;

#endif