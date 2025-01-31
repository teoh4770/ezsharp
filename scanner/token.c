#include <stdio.h>
#include "token.h"

Token makeToken(TokenType type, char *value, int line)
{
  Token token;
  token.type = type;
  token.value = value;
  token.line = line;

  return token;
}

void printToken(Token *token)
{
  printf("The token is at line %d\n", token->line);
  printf("The token type is %d\n", token->type);
  printf("The token value is %s\n", token->value);
}