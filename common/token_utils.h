// Define helper logic that operates on tokens

#ifndef TOKEN_UTILS
#define TOKEN_UTILS

#include "token.h"
#include <stdbool.h>

// Global variables
extern Token *look_ahead;

// Helper functions
void addEndToken(Token *tokens, int *tokenCount);
bool isAtEnd();
bool isKeyword(const char *keyword, int length);
bool isComparison(TokenType type);
bool isNumber(TokenType type);

// Main functions
void advanceToken();
Token *previousToken();
Token *nextToken();
bool matchToken(Token current, Token target);

#endif // TOKEN_UTILS