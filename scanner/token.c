#include <stdio.h>
#include "token.h"

//> make-token
Token makeToken(TokenType type, char *start, int length, int line)
{
  Token token;
  token.type = type;
  token.start = start;
  token.length = length;
  token.line = line;

  return token;
}
//< make-token

//> print-token
void printToken(Token *token)
{
  printf("The token starts at line %d\n", token->line);
  printf("The token type is %d\n", token->type);
  printf("length: %d\n", token->length);
  char *startCharacter = token->start;
  for (int i = 0; i < token->length; i++)
  {
    if (i == token->length - 1)
    {
      printf("%c\n", *startCharacter);
    }
    else
    {
      printf("%c", *startCharacter);
    }

    startCharacter++;
  }
}
//< print-token