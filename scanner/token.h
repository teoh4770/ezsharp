#ifndef TOKEN_H
#define TOKEN_H

typedef enum
{
  TOKEN_ADD,
  TOKEN_SUB,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_MOD,
  TOKEN_COMMA,
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_SQUARE_PAREN,
  TOKEN_RIGHT_SQUARE_PAREN,
  TOKEN_SEMICOLON,
  TOKEN_DOT,
  TOKEN_ASSIGN_OP,
  TOKEN_LT,
  TOKEN_LE,
  TOKEN_GT,
  TOKEN_GE,
  TOKEN_EQ,
  TOKEN_NE,
  TOKEN_INT,
  TOKEN_DOUBLE,
  TOKEN_ID,
  TOKEN_KEYWORD
} TokenType;

typedef struct
{
  TokenType type;
  char *value;
  int line;
} Token;

Token makeToken(TokenType type, char *value, int line);
void printToken(Token *token);

#endif