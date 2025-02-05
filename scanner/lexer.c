// Lexer.c: the main component to handle lexical analysis

#include <fcntl.h>  // For open() flags
#include <unistd.h> // For write() and close()
#include "lexer.h"

/**
 * stateToToken
 * Used to match the token when related state is found
 * e.g. if final state === STATE_LT, return TOKEN_LT
 */
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
    [STATE_WHITESPACE] = TOKEN_WHITESPACE,
    [STATE_ADD] = TOKEN_ADD,
    [STATE_MUL] = TOKEN_MUL,
    [STATE_DIV] = TOKEN_DIV,
    [STATE_MOD] = TOKEN_MOD,
    [STATE_SUB] = TOKEN_SUB,
    [STATE_INT] = TOKEN_INT,
    [STATE_DOUBLE] = TOKEN_DOUBLE,
    [STATE_NUM_E] = TOKEN_DOUBLE,
    [STATE_LEFT_SQUARE_PAREN] = TOKEN_LEFT_SQUARE_PAREN,
    [STATE_RIGHT_SQUARE_PAREN] = TOKEN_RIGHT_SQUARE_PAREN,
};

/**
 * keywords
 * A list of keywords that exists in our grammar
 * e.g. "or", "and", "not"...
 */
static const char *keywords[] = {
    "or", "and", "not", "if", "then", "else", "fi",
    "while", "do", "od", "def", "fed", "return",
    "print", "int", "double"};

static const int keywordsCount = sizeof(keywords) / sizeof(keywords[0]);

//> custom-string-functions

/**
 * stringLength
 * Find the size of a string, similar to strlen() in c
 * e.g. stringLength("abc") = 3
 */
size_t stringLength(const char *str)
{
  size_t length = 0;
  while (str[length] != '\0')
  {
    length++;
  }
  return length;
}

/**
 * stringCopy
 * Copy the string, similar to strcpy() in c
 * e.g. stringCopy(&word, "or")
 */
void stringCopy(char *dest, const char *src)
{
  while (*src)
  {
    *dest = *src;
    dest++;
    src++;
  }

  *dest = '\0';
}

/**
 * stringCompare
 * Compare both string, return 0 if string is identical
 * e.g. stringCompare("or", "or");
 */
int stringCompare(const char *str1, const char *str2)
{
  while (*str1 && (*str1 == *str2))
  {
    str1++;
    str2++;
  }

  // Return the ascii value of the first differing character (If any)
  return *str1 - *str2;
}

//< custom-string-functions

//> buffer-file-functions

void appendToBuffer(char *buffer, size_t *bufferIndex, const char *message)
{
  size_t messageLength = stringLength(message);

  // Handle buffer overflow? (later)
  if (*bufferIndex + messageLength >= BUFFER_SIZE - 1)
  {
    // flush the buffer earlier or log an error here
    return;
  }

  // Add message to the end of the buffer
  stringCopy(buffer + *bufferIndex, message);
  *bufferIndex += messageLength;
}

int generateFile(const char *fileName, const char *content)
{
  // O_WRONLY -> Writing only
  // O_CREAT  -> Create file if it does not exist
  // O_TRUNC  -> If the file already exists, it will clear its contents (truncate it to zero length).
  // S_IRUSR  -> Set the read permission for user
  // S_IWUSR  -> Set the write permission for user
  int file = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (file == -1)
  {
    perror("Failed to open file for writing");
    return -1;
  }

  printf("%s\n", content);

  size_t contentLength = stringLength(content);
  if (contentLength == 0)
  {
    close(file);
    return 0;
  }

  size_t bytesWrite = write(file, content, contentLength);
  if (bytesWrite == -1)
  {
    perror("Failed to write to file");
    close(file);
    return -1;
  }

  printf("Successfully writing file: %s\n", fileName);
  close(file);

  return 0;
}

void flushBufferToFile(const char *fileName, char *buffer, size_t *bufferIndex)
{
  generateFile(fileName, buffer);
  *bufferIndex = 0; // Reset buffer
}

//< buffer-file-functions

void initializeLexer(Lexer *lexer, int *inputFd, int *transitionTableFd)
{
  // Initialize lexer state and buffer-related variables
  lexer->currentState = STATE_START;
  lexer->tokenCount = 0;
  lexer->newLineCount = 0;
  lexer->characterCount = 0;
  lexer->hasNewLine = false;

  // Initialize double buffering for input reading
  initDoubleBuffer(&lexer->db, inputFd);
  fillBuffer(&lexer->db); // Fill the buffer with the first data chunk

  // Initialize the scanner
  initScanner(&lexer->scanner, lexer->db.buffer1);

  // Initialize the transition table with file data
  initTransitionTable(lexer->transitionTable, transitionTableFd);
}

void handleError(Lexer *lexer, char character)
{
  // Reset the state to start lexing from the invalid character
  lexer->currentState = STATE_START;

  // Move pointer to the invalid character to start from there
  TransitionState state = lexer->transitionTable[lexer->currentState][character];

  // If initial character is valid, then undo the col and forward pointer
  if (state != STATE_ERROR)
  {
    lexer->scanner.col--;     // Undo the column increment
    lexer->scanner.forward--; // Undo the last forward movement
  }

  lexer->scanner.lexemeBegin = lexer->scanner.forward;
}

Token getNextToken(TransitionState state, Scanner *scanner)
{
  // Map the state to the corresponding token type
  TokenType tokenType = stateToToken[state];

  int tokenLength = scanner->forward - scanner->lexemeBegin;
  char *startCharacter = scanner->lexemeBegin;

  // Get the token's lexeme
  char value[tokenLength + 1];
  for (int i = 0; i < tokenLength; i++)
  {
    value[i] = startCharacter[i];
  }
  value[tokenLength] = '\0';

  // If the token is of type keyword, check if the lexeme matches any predefined keywords
  if (tokenType == TOKEN_KEYWORD)
  {
    bool isKeyword = false;

    // Check if the lexeme matches any keyword
    for (int i = 0; i < keywordsCount; i++)
    {
      // if value inside keywords
      if (stringCompare(value, keywords[i]) == 0)
      {
        isKeyword = true;
      }
    }

    // If it is not a keyword, treat it as an identifier (ID)
    tokenType = isKeyword ? TOKEN_KEYWORD : TOKEN_ID;
  }

  return makeToken(tokenType, startCharacter, tokenLength, scanner->line);
}

void processToken(Lexer *lexer, TransitionState state)
{
  // Undo column and forward before processing the token
  lexer->scanner.col--;
  lexer->scanner.forward--;

  // Get the next token and add it to the token list
  Token token = getNextToken(state, &lexer->scanner);

  lexer->tokens[lexer->tokenCount++] = token;
  printToken(&token);

  // Prepare for the next token by updating lexemeBegin
  lexer->scanner.lexemeBegin = lexer->scanner.forward;

  // Reset line and column if a newline was encountered
  if (lexer->hasNewLine)
  {
    lexer->scanner.line = lexer->newLineCount + 1; // 1-based index for lines
    lexer->scanner.col = 0;                        // Reset column to 0 after newline
    lexer->hasNewLine = false;                     // Reset newline flag
  }
}

void lexicalAnalysis(int *inputFd, int *transitionTableFd)
{
  //> Output: tokens and errors log
  char errorBuffer[BUFFER_SIZE + 1]; // Holds error messages before writing to error file
  errorBuffer[BUFFER_SIZE] = '\0';   // Null character
  size_t errorBufferIndex = 0;       // Track the size of error buffer

  char tokenFileBuffer[BUFFER_SIZE + 1]; // Holds token info before writing to token file
  tokenFileBuffer[BUFFER_SIZE] = '\0';   // Null character
  size_t tokenFileBufferIndex = 0;
  //< Output

  Lexer lexer;
  initializeLexer(&lexer, inputFd, transitionTableFd);

  while (1)
  {
    char character = getNextChar(&lexer.db, &lexer.scanner);
    lexer.scanner.col++;

    // Handle end of file (EOF)
    if (character == EOF)
    {
      // Process the last token before finishing
      Token token = getNextToken(lexer.currentState, &lexer.scanner);

      // Token validation, possibly 0
      if (token.type <= 0)
      {
        break;
      }

      lexer.tokens[lexer.tokenCount++] = token;
      printToken(&token);
      break;
    }

    // Update lexer state based on the transition table
    TransitionState prevState = lexer.currentState;
    lexer.currentState = lexer.transitionTable[prevState][character];

    bool isTokenFound = lexer.currentState == STATE_START;
    bool isErrorFound = lexer.currentState == STATE_ERROR;

    // Handle error before processing token
    if (isErrorFound)
    {
      // Write error message
      char errorMessage[BUFFER_SIZE];
      sprintf(errorMessage, "Lexical Error: Unexpected character '%c' at line %d, column %d!\n",
              character, lexer.scanner.line, lexer.scanner.col);
      appendToBuffer(errorBuffer, &errorBufferIndex, errorMessage);

      handleError(&lexer, character);

      continue;
    }

    // Track character count only if we are still in a token
    if (!isTokenFound)
    {
      lexer.characterCount++;

      // Handle new line detection
      if (character == '\n')
      {
        lexer.newLineCount++;
        lexer.hasNewLine = true; // Remember that a newline appeared
      }
    }

    // Process a token if a valid token is found
    if (isTokenFound)
    {
      processToken(&lexer, prevState);
    }
  }

  // Handle Token file
  char tokenMessage[BUFFER_SIZE];

  for (int i = 0; i < lexer.tokenCount; i++)
  {
    Token token = lexer.tokens[i];

    // If token required attribute value
    if (token.type == TOKEN_KEYWORD || token.type == TOKEN_ID)
    {
      char tokenValue[token.length + 1]; // token value attribute is optional
      char *tokenStart = token.start;

      for (int i = 0; i < token.length; i++)
      {
        tokenValue[i] = tokenStart[i];
      }
      tokenValue[token.length] = '\0';

      sprintf(tokenMessage, "%d %s\n", token.type, tokenValue);
    }
    else
    {
      sprintf(tokenMessage, "%d\n", token.type);
    }

    appendToBuffer(tokenFileBuffer, &tokenFileBufferIndex, tokenMessage);
  }

  // Flush buffer contents to the file at the end of lexical analysis
  flushBufferToFile("lexical_analysis_errors.txt", errorBuffer, &errorBufferIndex);
  flushBufferToFile("token_lexeme_pairs.txt", tokenFileBuffer, &tokenFileBufferIndex);
}