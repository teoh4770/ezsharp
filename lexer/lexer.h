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

static Token tokens[1024];
static int tokenCount = 0;

typedef struct
{
  TransitionState currentState;
  TransitionState transitionTable[TT_ROWS][TT_COLS];
  DoubleBuffer db;
  Scanner scanner;
  int newLineCount;
  int characterCount;
  bool hasNewLine;
} Lexer;

Token *lexicalAnalysis(int *inputFd, int *transitionTableFd);
int getTokenCount();

#endif