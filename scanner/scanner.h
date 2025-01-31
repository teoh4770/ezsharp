#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "buffer.h"
#include "token.h"
#include "transition_table.h"

typedef struct
{
  char *lexemeBegin;
  char *forward;
  int line;
} Scanner;

void initScanner(Scanner *scanner, char *buffer);
char peek(Scanner *scanner);
char getNextChar(DoubleBuffer *db, Scanner *scanner);

#endif