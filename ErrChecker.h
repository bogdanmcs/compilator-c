#ifndef ERRCHECKER_H
#define ERRCHECKER_H
#include "Token.h"

void err(const char *fmt,...);
void tkerr(const Token *tk,const char *fmt,...);


#endif