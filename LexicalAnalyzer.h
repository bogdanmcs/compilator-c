#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H
#include "Token.h"

char* createString(const char *strInt, char *strEnd);
double createDouble(const char *strInt, char *strEnd);
long int createLongInt(const char *strInt, char *strEnd);
long int createLongIntChar(const char *strInt, char *strEnd);

void initToken();
Token* addTk(int code, int line);
int getNextToken();
void freeMem();
void showAtoms();
void openFileAndSetPointer(char *filename);
Token* analyzeLex(char *fileName);

#endif