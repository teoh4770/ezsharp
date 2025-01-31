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
      [STATE_WHITESPACE] = TOKEN_WHITESPACE, // Handle separately if needed
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
    // log error here
    //
  }

  // Todo: parse the lexeme value here
  TokenType tokenType = stateToToken[state];
  int length = scanner->forward - scanner->lexemeBegin;
  char tokenValue[length];
  char *startCharacter = scanner->lexemeBegin;

  // if tokenType is keyword, check if the attribute value match any keyword in our keyword list, if not, return invalid

  return makeToken(tokenType, startCharacter, length, scanner->line);
}
//< get-next-token

void lexicalAnalysis(int *inputFd, int *transitionTableFd)
{
  // Scan through all the characters
  // Output a list of token-lexeme pairs file and an error log file

  TransitionState state = STATE_START;
  int transitionTable[TT_ROWS][TT_COLS] = {0};
  DoubleBuffer db;
  Scanner scanner;

  initDoubleBuffer(&db, inputFd);
  fillBuffer(&db);
  initScanner(&scanner, db.buffer1);
  initTransitionTable(TT_ROWS, TT_COLS, transitionTable, transitionTableFd);

  int hasNewLine = 0;
  int newLineCount = 0;
  int tokenCount = 0;
  int characterCount = 0;
  Token tokens[100] = {0};

  while (1)
  {
    char character = getNextChar(&db, &scanner);
    characterCount++;

    // Handle EOF
    if (character == EOF)
    {
      // Note: need to get the last token
      Token token = getNextToken(state, &scanner);
      tokens[tokenCount++] = token;
      printToken(&token);

      break;
    }

    TransitionState prevState = state;
    state = transitionTable[state][character];

    puts("==============================");
    printf("Char count: %d\n", characterCount);
    printf("Current char: %c\n", character);
    printf("prev state: %d\n", prevState);
    printf("curr state: %d\n", state);

    // When state not start state, means that we still forwarding
    // thus increment the newline count
    if (character == '\n' && state != STATE_START)
    {
      newLineCount++;
      hasNewLine = 1;
    }

    // Get token
    if (prevState != state && state == STATE_START)
    {
      scanner.forward--;
      Token token = getNextToken(prevState, &scanner);
      tokens[tokenCount++] = token;
      printToken(&token);
      scanner.lexemeBegin = scanner.forward;

      if (token.type == TOKEN_WHITESPACE && hasNewLine) {
        scanner.line = newLineCount + 1; // 1 based
        hasNewLine = 0;
      }
    }
  }
}
