#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  TOKEN_ADD = 0,
  TOKEN_SUB = 1,
  TOKEN_MUL = 2,
  TOKEN_DIV = 3,
  TOKEN_MOD = 4,
  TOKEN_COMMA = 5,
  TOKEN_LEFT_PAREN = 6,
  TOKEN_RIGHT_PAREN = 7,
  TOKEN_LEFT_SQUARE_PAREN = 8,
  TOKEN_RIGHT_SQUARE_PAREN = 9,
  TOKEN_SEMICOLON = 10,
  TOKEN_DOT = 11,
  TOKEN_ASSIGN_OP = 12,
  TOKEN_LT = 13,
  TOKEN_LE = 14,
  TOKEN_GT = 15,
  TOKEN_GE = 16,
  TOKEN_EQ = 17,
  TOKEN_NE = 18,
  TOKEN_INT = 19,
  TOKEN_DOUBLE = 20,
  TOKEN_ID = 21,
  TOKEN_KEYWORD = 22,
  TOKEN_WHITESPACE = 23,
  TOKEN_DOLLAR = 24
} TokenType;

typedef struct {
  TokenType type;
  char *start; // Start of lexeme
  int length;  // Size of lexeme
  int line;
  char *lexeme;
} Token;

Token makeToken(TokenType type, char *start, int length, int line);
void printToken(Token *token);
char *getTokenLexeme(Token *token);

#endif