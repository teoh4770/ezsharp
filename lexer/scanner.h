#ifndef SCANNER_H
#define SCANNER_H

#include "../common/token.h"
#include "buffer.h"
#include "transition_table.h"
#include <stdio.h>

typedef struct {
  char *lexemeBegin;
  char *forward;
  int line;
  int col;
} Scanner;

void initScanner(Scanner *scanner, char *buffer);
char peek(Scanner *scanner);
char getNextChar(DoubleBuffer *db, Scanner *scanner);

#endif