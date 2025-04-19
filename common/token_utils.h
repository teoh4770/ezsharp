// Helper logic that operates on tokens

#ifndef TOKEN_UTILS_H
#define TOKEN_UTILS_H

#include "token.h"
#include <stdbool.h>

// Global variables
extern Token *look_ahead;

// Helper functions
bool isAtEnd();

// Classification
bool isKeyword(const char *keyword, int length);
bool isComparison(TokenType type);
bool isNumber(TokenType type);

// Navigation
void advanceToken();
Token *previousToken();
Token *nextToken();

// Matching
bool matchType(TokenType expectedType);
bool matchKeyword(const char *expectedKeyword);

#endif // TOKEN_UTILS