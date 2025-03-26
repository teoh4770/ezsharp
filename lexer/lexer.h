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

//> file functions (need to move to common folder)
void appendToBuffer(char *buffer, size_t *bufferIndex, const char *message);
int generateFile(const char *fileName, const char *content);
void flushBufferToFile(const char *fileName, char *buffer, size_t *bufferIndex);
//< file functions

Token *lexicalAnalysis(int *inputFd, int *transitionTableFd);
int getTokenCount();

#endif