#include <stdio.h>
#include <stdlib.h>
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

void printTokenLexeme(Token *token)
{
  char *startCharacter = token->start;
  for (int i = 0; i < token->length; i++)
  {
    if (i == token->length - 1)
    {
      printf("%c", *startCharacter);
    }
    else
    {
      printf("%c", *startCharacter);
    }

    startCharacter++;
  }
}

char *getTokenLexeme(Token *token)
{
  char *lexeme = (char *)malloc(token->length + 1);
  if (!lexeme)
  {
    printf("Memory allocation failed\n");
    return NULL;
  }

  for (int i = 0; i < token->length; i++)
  {
    lexeme[i] = token->start[i];
  }

  lexeme[token->length] = '\0';

  return lexeme;
}