#include "token_utils.h"
#include "../common/string.h"
#include <stdio.h>

// Global variables
Token *look_ahead = NULL;

// Helper functions
void addEndToken(Token *tokens, int *tokenCount) {
  tokens[*tokenCount] = makeToken(TOKEN_DOLLAR, "$", 1, -1);
  (*tokenCount)++;
}

bool isAtEnd() { return look_ahead->type == TOKEN_DOLLAR; }

bool isKeyword(const char *keyword, int length) {
  return look_ahead->type == TOKEN_KEYWORD &&
         _strncmp(look_ahead->lexeme, keyword, length) == 0;
}

bool isComparison(TokenType type) {
  return type == TOKEN_LT || type == TOKEN_LE || type == TOKEN_GT ||
         type == TOKEN_GE || type == TOKEN_EQ || type == TOKEN_NE;
}

bool isNumber(TokenType type) {
  return type == TOKEN_INT || type == TOKEN_DOUBLE;
}

// Main functions
void advanceToken() {
  if (look_ahead->type != TOKEN_DOLLAR) {
    look_ahead++;
  }
}

Token *previousToken() { return look_ahead - 1; }

Token *nextToken() { return look_ahead + 1; }

bool matchToken(Token current, Token target) {
  puts("================");
  puts("Look-ahead Token");
  puts("================");
  printToken(&current);

  puts("============");
  puts("Target Token");
  puts("============");
  printToken(&target);

  bool isTokenTypeMatched = current.type == target.type;
  bool isTokenLexemeMatched =
      _strncmp(current.lexeme, target.lexeme, current.length) == 0;

  if (!isTokenTypeMatched || !isTokenLexemeMatched) {
    puts("-> Token Not Match\n");
    return false;
  }

  puts("-> Token Match\n");
  return true;
}
