#include "token_utils.h"
#include "../common/string.h"
#include <stdio.h>

Token *look_ahead = NULL;

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

void advanceToken() {
  if (look_ahead->type != TOKEN_DOLLAR) {
    look_ahead++;
  }
}

Token *previousToken() { return look_ahead - 1; }

Token *nextToken() { return look_ahead + 1; }

bool matchType(TokenType expectedType) {
  puts("================");
  puts("Look-ahead Token");
  puts("================");
  printToken(look_ahead);

  if (look_ahead->type != expectedType) {
    puts("==> Incorrect Type");
    printf("==> Expected Type is %d\n\n", expectedType);
    return false;
  }

  puts("==> Correct Type\n");
  advanceToken();
  return true;
}

bool matchKeyword(const char *expectedKeyword) {
  puts("================");
  puts("Look-ahead Token");
  puts("================");
  printToken(look_ahead);

  if (_strcmp(look_ahead->lexeme, expectedKeyword) != 0) {
    puts("==>  Incorrect Keyword");
    printf("==> Expected Keyword is %s\n\n", expectedKeyword);
    return false;
  }

  puts("==>  Correct Keyword\n");
  advanceToken();
  return true;
}