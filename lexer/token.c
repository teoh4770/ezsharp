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
  token.lexeme = getTokenLexeme(&token);

  return token;
}
//< make-token

//> print-token
void printToken(Token *token)
{
  printf("The token starts at line %d\n", token->line);
  printf("The token type is %d\n", token->type);
  printf("The token lexeme size is: %d\n", token->length);
  printf("The token lexeme: %s\n", token->lexeme);
}
//< print-token

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