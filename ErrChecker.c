#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Token.h"
#include "ErrChecker.h"


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