#include "token.h"
#include "../common/string.h"
#include <stdio.h>
#include <stdlib.h>

//> make-token
Token makeToken(TokenType type, char *start, int length, int line) {
  Token token;
  token.type = type;
  token.start = start;
  token.length = length;
  token.line = line;
  token.lexeme = malloc(length + 1);
  _strncpy(token.lexeme, start, length);
  token.lexeme[length] = '\0';

  return token;
}
//< make-token

//> print-token
void printToken(Token *token) {
  printf("Token Line   : %d\n", token->line);
  printf("Token Type   : %d\n", token->type);
  printf("Lexeme Size  : %d\n", token->length);
  printf("Token Lexeme : %s\n", token->lexeme);
}
//< print-token

char *getTokenLexeme(Token *token) {
  char *lexeme = (char *)malloc(token->length + 1);

  if (!lexeme) {
    printf("Memory allocation failed\n");
    return NULL;
  }

  _strncpy(lexeme, token->lexeme, token->length);

  lexeme[token->length] = '\0';

  return lexeme;
}