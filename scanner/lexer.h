#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include "scanner.h"
#include "token.h"
#include "transition_table.h"

typedef enum
{
  false,
  true
} bool;

typedef struct
{
  TransitionState currentState;
  TransitionState transitionTable[TT_ROWS][TT_COLS];
  DoubleBuffer db;
  Scanner scanner;
  Token tokens[1024]; 
  int tokenCount;
  int newLineCount;
  int characterCount;
  bool hasNewLine;
} Lexer;

void lexicalAnalysis(int *inputFd, int *transitionTableFd);

#endif