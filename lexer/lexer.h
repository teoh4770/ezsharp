#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include "../common/token.h"
#include "scanner.h"
#include "stdbool.h"
#include "transition_table.h"

Token tokens[1024];
extern int tokenCount;

typedef struct {
  TransitionState currentState;
  TransitionState transitionTable[TT_ROWS][TT_COLS];
  DoubleBuffer db;
  Scanner scanner;
  int newLineCount;
  int characterCount;
  bool hasNewLine;
} Lexer;

Token *lexicalAnalysis(int *inputFd, int *transitionTableFd);

#endif