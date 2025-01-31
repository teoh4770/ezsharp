#include "lexer.h"
#include "scanner.h"
#include "token.h"
#include "transition_table.h"

//> get-next-token
Token getNextToken(TransitionState state, Scanner *scanner)
{
  static const TokenType stateToToken[] = {
      [STATE_LT] = TOKEN_LT,
      [STATE_LE] = TOKEN_LE,
      [STATE_NE] = TOKEN_NE,
      [STATE_ASSIGN_OP] = TOKEN_ASSIGN_OP,
      [STATE_EQ] = TOKEN_EQ,
      [STATE_GT] = TOKEN_GT,
      [STATE_GE] = TOKEN_GE,
      [STATE_KEYWORD] = TOKEN_KEYWORD,
      [STATE_ID] = TOKEN_ID,
      [STATE_SEMICOLON] = TOKEN_SEMICOLON,
      [STATE_COMMA] = TOKEN_COMMA,
      [STATE_LEFT_PAREN] = TOKEN_LEFT_PAREN,
      [STATE_RIGHT_PAREN] = TOKEN_RIGHT_PAREN,
      [STATE_DOT] = TOKEN_DOT,
      // [STATE_WHITESPACE] = TOKEN_WHITESPACE, // Handle separately if needed
      [STATE_ADD] = TOKEN_ADD,
      [STATE_MUL] = TOKEN_MUL,
      [STATE_DIV] = TOKEN_DIV,
      [STATE_MOD] = TOKEN_MOD,
      [STATE_SUB] = TOKEN_SUB,
      [STATE_INT] = TOKEN_INT,
      [STATE_DOUBLE] = TOKEN_DOUBLE,
      [STATE_NUM_E] = TOKEN_DOUBLE, // Assuming exponential notation
      [STATE_LEFT_SQUARE_PAREN] = TOKEN_LEFT_SQUARE_PAREN,
      [STATE_RIGHT_SQUARE_PAREN] = TOKEN_RIGHT_SQUARE_PAREN,
  };

  if (state < 0)
  {
    // handle error here
  }

  // Todo: parse the lexeme value here
  TokenType tokenType = stateToToken[state];
  int length = scanner->forward - scanner->lexemeBegin;
  char tokenValue[length];

  // if tokenType is keyword, check if the attribute value match any keyword in our keyword list, if not, return invalid

  return makeToken(tokenType, "tokenValue", scanner->line);
}
//< get-next-token

void lexicalAnalysis(int *inputFd, int *transitionTableFd)
{
  TransitionState state = STATE_START;
  int transitionTable[TT_ROWS][TT_COLS] = {0};
  DoubleBuffer db;
  Scanner scanner;

  initTransitionTable(TT_ROWS, TT_COLS, transitionTable, transitionTableFd);
  initDoubleBuffer(&db, inputFd);
  initScanner(&scanner, db.buffer1);
  fillBuffer(&db);

  int tokenCount = 0;
  Token tokens[100] = {0};

  while (1)
  {
    // we always check for white space at the beginning, and skip them if possible
    if (
        peek(&scanner) == '\n' ||
        peek(&scanner) == ' ' ||
        peek(&scanner) == '\t')
    {
      getNextChar(&db, &scanner);
    }

    char c = getNextChar(&db, &scanner);

    // Handling special cases first, like eof and whitespace
    if (c == EOF)
    {
      // Get the last token of the file here
      Token token = getNextToken(state, &scanner);
      tokens[tokenCount++] = token;
      break;
    }

    TransitionState prevState = state;
    state = transitionTable[state][c];

    // Handle token extraction when transitioning back to STATE_START
    if (prevState != state && state == STATE_START)
    {
      // If lexeme is determined, forward is set to the character at its right end
      scanner.forward--;
      // then after getting the token
      Token token = getNextToken(prevState, &scanner);
      tokens[tokenCount++] = token;
      printToken(&token);
      // lexemeBegin sets to the character immediately after the lexeme just found
      scanner.lexemeBegin = scanner.forward;
    }

    // Handle error state
  }
}
