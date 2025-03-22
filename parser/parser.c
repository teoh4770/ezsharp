// parser.c: the main component to handle parsing

/*
Parser Input: a list of token from lexer (token_lexeme_pairs.txt)

Output
1. An error log file with diagnostics (if syntax error occurs) (If the source code was syntactically correct, the error log file would be empty.)
2. After parsing is done, you program should print symbol table in a readable format; (for parser, printing out just all the identifiers from the source code) (required)
3. Test your implementation on test cases provided on MyLS, also create your own tests.
*/

#include <stdio.h>
#include <fcntl.h>  // For open() flags
#include <unistd.h> // For write() and close()

#include "parser.h"

//> Helper Functions
void addEndToken(Token *tokens, int *tokenCount)
{
  tokens[*tokenCount] = makeToken(TOKEN_DOLLAR, "$", 1, -1);
  (*tokenCount)++;
}

void preParse(const char *message)
{
  printf("Currently parsing: %s\n", message);
}

void advanceToken()
{
  if (look_ahead->type != TOKEN_DOLLAR)
  {
    look_ahead++;
  }
}

// print and create symbol table file
void printSymbolTable()
{
  puts("======================");
  puts("Print out symbol table");
  puts("======================");

  char symbolTable[BUFFER_SIZE + 1];
  symbolTable[BUFFER_SIZE] = '\0';

  for (int i = 0; i < identifierCount; i++)
  {
    sprintf(symbolTable, "%s\n", getTokenLexeme(&identifiers[i]));
    appendToBuffer(symbolTableBuffer, &symbolTableBufferIndex, symbolTable);
  }

  flushBufferToFile("symbol_table.txt", symbolTableBuffer, &symbolTableBufferIndex);
}

// print and create syntax error file
void syntaxError(const char *expectedMessage)
{
  char *lexeme = getTokenLexeme(look_ahead);

  char syntaxErrorMessage[BUFFER_SIZE + 1];
  syntaxErrorMessage[BUFFER_SIZE] = '\0';
  sprintf(syntaxErrorMessage, "Syntax Error: %s, but found '%s' at line %d\n", expectedMessage, lexeme, look_ahead->line);

  appendToBuffer(syntaxErrorBuffer, &syntaxErrorBufferIndex, syntaxErrorMessage);
  flushBufferToFile("syntax_analysis_errors.txt", syntaxErrorBuffer, &syntaxErrorBufferIndex);
}

bool matchToken(Token current, Token target)
{
  puts("================");
  puts("Look-ahead Token");
  puts("================");
  printToken(&current);

  puts("============");
  puts("Target Token");
  puts("============");
  printToken(&target);

  // Compare token type and token length
  if (current.type != target.type || current.length != target.length)
  {
    puts("-> Token Not Match\n");
    return false;
  }

  // Compare lexeme
  if (stringCompare(current.start, target.start, current.length) != 0)
  {
    puts("-> Token Not Match\n");
    return false;
  }

  puts("-> Token Match\n");
  return true;
};

bool isAtEnd()
{
  return look_ahead->type == TOKEN_DOLLAR;
}

//< Helper functions

//> Parse Functions
bool match(Token token)
{
  // If token matched
  // Update our symbol table
  // Move to the next token
  if (matchToken(*look_ahead, token))
  {
    if (look_ahead->type == TOKEN_ID)
    {
      identifiers[identifierCount++] = *look_ahead;
    }

    advanceToken();

    return true;
  }

  // printf("Error: Unexpected token %s\n", look_ahead->start); // Error message
  return false;
}

void Parse(Token *tokens, int tokenCount)
{
  puts("===============");
  puts("Start parsing!");
  puts("===============");

  // Remove the created files first
  remove("syntax_analysis_errors.txt");
  remove("symbol_table.txt");

  // Initialization
  syntaxErrorBuffer[BUFFER_SIZE] = '\0';
  symbolTableBuffer[BUFFER_SIZE] = '\0';
  // Add extra $ token to indicate end of input tokens
  addEndToken(tokens, &tokenCount);
  // Initialize the look ahead variable
  look_ahead = tokens;

  // Start Parsing, with parseProg as the starting function in Parse()
  parseProg();

  if (look_ahead->type == TOKEN_DOLLAR)
  {
    puts("=====================");
    puts("Parsing Reach To End!");
    puts("=====================");

    printSymbolTable();
  }
  else
  {
    puts("=========================");
    puts("Parsing Not Reach To End!");
    puts("=========================");
  }
}

// !remove
// void syncProg()
// {
//   while (!isAtEnd())
//   {
//     advanceToken();
//   }
// }

void parseProg()
{
  // PROG → FNS DECLS STMTS .
  preParse("prog");

  parseFns();
  parseDecls();
  parseStmts();

  if (!match(makeToken(TOKEN_DOT, ".", 1, -1)))
  {
    syntaxError("Expected '.' to indicate end of the program");

    while (!isAtEnd())
    {
      advanceToken();
    }

    return;
  }
}

// !remove
// void syncFns()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_DOT ||
//         look_ahead->type == TOKEN_SEMICOLON ||
//         look_ahead->type == TOKEN_ID ||
//         look_ahead->type == TOKEN_INT ||
//         look_ahead->type == TOKEN_DOUBLE ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "if", 2) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "while", 5) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "print", 5) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "return", 6) == 0)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseFns()
{
  // FNS → FN ; FNSC
  // FNS → ε
  preParse("fns");

  bool isKeywordDef = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "def", 3) == 0;

  if (isKeywordDef)
  {
    parseFn();

    if (!match(makeToken(TOKEN_SEMICOLON, ";", 1, -1)))
    {
      syntaxError("Expected ';' after function definition");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_ID ||
          look_ahead->type == TOKEN_INT ||
          look_ahead->type == TOKEN_DOUBLE ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "if", 2) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "while", 5) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "print", 5) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "return", 6) == 0)
      {
        return;
      }

      advanceToken();
      parseFns();
    }

    parseFnsc();
  }
  else
  {
    return;
  }
}

void parseFnsc()
{
  // FNSC → FN; FNSC
  // FNSC → ε
  preParse("fnsc");
  parseFns();
}

// !remove
// void syncFn()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_SEMICOLON)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseFn()
{
  // FN → def TYPE FNAME ( PARAMS ) DECLS STMTS fed
  preParse("fn");

  if (!match(makeToken(TOKEN_KEYWORD, "def", 3, -1)))
  {
    syntaxError("Expected 'def' at the start of function definition");

    if (look_ahead->type == TOKEN_SEMICOLON)
    {
      return;
    }

    advanceToken();
    parseFn();
  }

  parseType();
  parseFname();

  if (!match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1)))
  {
    syntaxError("Expected '(' after function name");

    if (look_ahead->type == TOKEN_SEMICOLON)
    {
      return;
    }

    advanceToken();
    parseFn();
  }

  parseParams();

  if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
  {
    syntaxError("Expected ')' after function parameters");

    if (look_ahead->type == TOKEN_SEMICOLON)
    {
      return;
    }

    advanceToken();
    parseFn();
  }

  parseDecls();
  parseStmts();

  if (!match(makeToken(TOKEN_KEYWORD, "fed", 3, -1)))
  {
    syntaxError("Expected 'fed' at the end of function definition");

    if (look_ahead->type == TOKEN_SEMICOLON)
    {
      return;
    }

    advanceToken();
    parseFn();
  }
}

void parseParams()
{
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  preParse("params");

  if (look_ahead->type == TOKEN_KEYWORD)
  {
    if (stringCompare(look_ahead->start, "int", 3) == 0 ||
        stringCompare(look_ahead->start, "double", 6) == 0)
    {
      parseType();
      parseVar();
      parseParamsc();
    }
  }
  else
  {
    return;
  }
}

// !remove
// void syncParamsc()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_RIGHT_PAREN)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseParamsc()
{
  // PARAMSC → , <type> <var> <paramsc> | ε
  preParse("paramsc");

  if (look_ahead->type == TOKEN_COMMA)
  {
    match(makeToken(TOKEN_COMMA, ",", 1, -1));

    if (!(look_ahead->type == TOKEN_KEYWORD &&
          (stringCompare(look_ahead->start, "int", 3) == 0 ||
           stringCompare(look_ahead->start, "double", 6) == 0)))
    {
      syntaxError("Expected a type ('int' or 'double') after ',' in parameter list");

      if (look_ahead->type == TOKEN_RIGHT_PAREN)
      {
        return;
      }

      advanceToken();
      parseParamsc();
    }

    parseType();
    parseVar();
    parseParamsc();
  }
  else
  {
    return;
  }
}

// !remove
// void syncFname()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_LEFT_PAREN)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseFname()
{
  // FNAME → ID
  preParse("fname");

  if (look_ahead->type == TOKEN_ID)
  {
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
  }
  else
  {
    syntaxError("Expected function name (identifier)");

    if (look_ahead->type == TOKEN_LEFT_PAREN)
    {
      return;
    }

    advanceToken();
    parseFname();
  }
}

// !remove
// void syncDecls()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_DOT ||
//         look_ahead->type == TOKEN_SEMICOLON ||
//         look_ahead->type == TOKEN_ID ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "fed", 3) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "if", 2) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "while", 5) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "print", 5) == 0 ||
//         look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "return", 6) == 0)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseDecls()
{
  // DECLS → DECL; DECLSC
  // DECLS → ε
  preParse("decls");

  bool isKeywordInt = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "int", 3) == 0;
  bool isKeywordDouble = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "double", 6) == 0;

  if (isKeywordInt || isKeywordDouble)
  {
    parseDecl();

    if (!match(makeToken(TOKEN_SEMICOLON, ";", 1, -1)))
    {
      syntaxError("Expected semicolon");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_ID ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "fed", 3) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "if", 2) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "while", 5) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "print", 5) == 0 ||
          look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "return", 6) == 0)
      {
        return;
      }

      advanceToken();
      parseDecls();
    }

    parseDeclsc();
  }
  else
  {
    return;
  }
}

void parseDeclsc()
{
  // DECLSC → DECL; DECLSC
  // DECLSC → ε
  preParse("declsc");
  parseDecls();
}

// void syncDecl()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_SEMICOLON)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseDecl()
{
  // DECL → TYPE VARS
  preParse("decl");

  bool isKeywordInt = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "int", 3) == 0;
  bool isKeywordDouble = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "double", 6) == 0;

  if (isKeywordInt || isKeywordDouble)
  {
    parseType();
    parseVars();
  }
  else
  {
    syntaxError("Expected 'int' or 'double' for declaration");

    if (look_ahead->type == TOKEN_SEMICOLON)
    {
      return;
    }

    advanceToken();
    parseDecl();
  }
}

// !remove
// void syncType()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_ID)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseType()
{
  // TYPE → int
  // TYPE → double
  preParse("type");

  bool isKeywordInt = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "int", 3) == 0;
  bool isKeywordDouble = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "double", 6) == 0;

  if (isKeywordInt)
  {
    match(makeToken(TOKEN_KEYWORD, "int", 3, -1));
  }
  else if (isKeywordDouble)
  {
    match(makeToken(TOKEN_KEYWORD, "double", 6, -1));
  }
  else
  {
    syntaxError("Expected 'int' or 'double' as type");

    if (look_ahead->type == TOKEN_ID)
    {
      return;
    }

    advanceToken();
    parseType();
  }
}

void parseVars()
{
  // VARS → VAR VARSC
  preParse("vars");

  parseVar();
  parseVarsc();
}

void parseVarsc()
{
  // VARSC → , VARS
  // VARSC → ε
  preParse("varsc");

  if (look_ahead->type == TOKEN_COMMA)
  {
    match(makeToken(TOKEN_COMMA, ",", 1, -1));
    parseVars();
  }
  else
  {
    return;
  }
}

void parseStmts()
{
  // STMTS → STMT STMTSC
  preParse("stmts");

  parseStmt();
  parseStmtsc();
}

void parseStmtsc()
{
  // STMTSC → ; STMTS
  // STMTSC → ε
  preParse("stmtsc");

  if (look_ahead->type == TOKEN_SEMICOLON)
  {
    match(makeToken(TOKEN_SEMICOLON, ";", 1, -1));
    parseStmts();
  }
  else
  {
    return;
  }
}

// !remove
// void syncStmt()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_DOT ||
//         look_ahead->type == TOKEN_SEMICOLON ||
//         look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
//                                               stringCompare(look_ahead->start, "od", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "fi", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "else", 4) == 0))
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseStmt()
{
  // STMT → VAR = EXPR
  // STMT → if BEXPR then STMTS STMTC
  // STMT → while BEXPR do STMTS od
  // STMT → print EXPR
  // STMT -> return EXPR
  // STMT →  ε
  preParse("stmt");

  bool isKeywordIf = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "if", 2) == 0;
  bool isKeywordWhile = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "while", 5) == 0;
  bool isKeywordPrint = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "print", 5) == 0;
  bool isKeywordReturn = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "return", 6) == 0;

  if (look_ahead->type == TOKEN_ID)
  {
    parseVar();

    if (!match(makeToken(TOKEN_ASSIGN_OP, "=", 1, -1)))
    {
      syntaxError("Expected '=' for assignment");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0))
      {
        return;
      }

      advanceToken();
      parseStmt();
    }

    parseExpr();
  }
  else if (isKeywordIf)
  {
    match(makeToken(TOKEN_KEYWORD, "if", 2, -1));
    parseBexpr();

    if (!match(makeToken(TOKEN_KEYWORD, "then", 4, -1)))
    {
      syntaxError("Missing 'then' after 'if' condition");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0))
      {
        return;
      }

      advanceToken();
      parseStmt();
    }

    parseStmts();
    parseStmtc();
  }
  else if (isKeywordWhile)
  {
    match(makeToken(TOKEN_KEYWORD, "while", 5, -1));
    parseBexpr();

    if (!match(makeToken(TOKEN_KEYWORD, "do", 2, -1)))
    {
      syntaxError("Missing 'do' after 'while' condition");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0))
      {
        return;
      }

      advanceToken();
      parseStmt();
    }

    parseStmts();

    if (!match(makeToken(TOKEN_KEYWORD, "od", 2, -1)))
    {
      syntaxError("Expected 'od' at the end of while loop");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0))
      {
        return;
      }

      advanceToken();
      parseStmt();
    }
  }
  else if (isKeywordPrint)
  {
    match(makeToken(TOKEN_KEYWORD, "print", 5, -1));
    parseExpr();
  }
  else if (isKeywordReturn)
  {
    match(makeToken(TOKEN_KEYWORD, "return", 6, -1));
    parseExpr();
  }
  else
  {
    return;
  }
}

// !remove
// void syncStmtc()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_DOT ||
//         look_ahead->type == TOKEN_SEMICOLON ||
//         look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
//                                               stringCompare(look_ahead->start, "od", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "fi", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "else", 4) == 0))
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseStmtc()
{
  // STMTC → fi
  // STMTC → else STMTS fi
  preParse("stmtc");

  bool isKeywordFi = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "fi", 2) == 0;
  bool isKeywordElse = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "else", 4) == 0;

  if (isKeywordFi)
  {
    match(makeToken(TOKEN_KEYWORD, "fi", 2, -1));
  }
  else if (isKeywordElse)
  {
    match(makeToken(TOKEN_KEYWORD, "else", 4, -1));
    parseStmts();

    if (!match(makeToken(TOKEN_KEYWORD, "fi", 2, -1)))
    {
      syntaxError("Statement does not end with 'fi'");

      if (look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0))
      {
        return;
      }

      advanceToken();
      parseStmtc();
    }
  }
  else
  {
    syntaxError("Expected 'fi' or 'else' for the end of statement");

    if (look_ahead->type == TOKEN_DOT ||
        look_ahead->type == TOKEN_SEMICOLON ||
        look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                              stringCompare(look_ahead->start, "od", 2) == 0 ||
                                              stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                              stringCompare(look_ahead->start, "else", 4) == 0))
    {
      return;
    }

    advanceToken();
    parseStmtc();
  }
}

void parseExpr()
{
  // EXPR → TERM EXPRC
  preParse("expr");

  parseTerm();
  parseExprc();
}

void parseExprc()
{
  // EXPRC → + TERM EXPRC
  // EXPRC → - TERM EXPRC
  // EXPRC → ε
  preParse("exprc");

  if (look_ahead->type == TOKEN_ADD)
  {
    match(makeToken(TOKEN_ADD, "+", 1, -1));
    parseTerm();
    parseExprc();
  }
  else if (look_ahead->type == TOKEN_SUB)
  {
    match(makeToken(TOKEN_SUB, "-", 1, -1));
    parseTerm();
    parseExprc();
  }
  else
  {
    return;
  }
}

void parseTerm()
{
  // TERM → FACTOR TERMC
  preParse("term");

  parseFactor();
  parseTermc();
}

void parseTermc()
{
  // TERMC → * FACTOR TERMC
  // TERMC → / FACTOR TERMC
  // TERMC → % FACTOR TERMC
  // TERMC → ε

  preParse("termc");

  if (look_ahead->type == TOKEN_MUL)
  {
    match(makeToken(TOKEN_MUL, "*", 1, -1));
    parseFactor();
    parseTermc();
  }
  else if (look_ahead->type == TOKEN_DIV)
  {
    match(makeToken(TOKEN_DIV, "/", 1, -1));
    parseFactor();
    parseTermc();
  }
  else if (look_ahead->type == TOKEN_MOD)
  {
    match(makeToken(TOKEN_MOD, "%", 1, -1));
    parseFactor();
    parseTermc();
  }
  else
  {
    return;
  }
}

// !remove
// void syncFactor()
// {
//   while (!isAtEnd())
//   {
//     if (look_ahead->type == TOKEN_DOT ||
//         look_ahead->type == TOKEN_SEMICOLON ||
//         look_ahead->type == TOKEN_RIGHT_PAREN ||
//         look_ahead->type == TOKEN_COMMA ||
//         look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
//                                               stringCompare(look_ahead->start, "od", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "fi", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "else", 4) == 0) ||
//         look_ahead->type == TOKEN_ADD ||
//         look_ahead->type == TOKEN_SUB ||
//         look_ahead->type == TOKEN_MUL ||
//         look_ahead->type == TOKEN_DIV ||
//         look_ahead->type == TOKEN_MOD ||
//         look_ahead->type == TOKEN_LT ||
//         look_ahead->type == TOKEN_GT ||
//         look_ahead->type == TOKEN_EQ ||
//         look_ahead->type == TOKEN_LE ||
//         look_ahead->type == TOKEN_GE ||
//         look_ahead->type == TOKEN_NE ||
//         look_ahead->type == TOKEN_RIGHT_SQUARE_PAREN)
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseFactor()
{
  // FACTOR → ID FACTORC
  // FACTOR → NUMBER
  // FACTOR → (EXPR)
  preParse("factor");

  bool isNumber = look_ahead->type == TOKEN_INT ||
                  look_ahead->type == TOKEN_DOUBLE;

  if (look_ahead->type == TOKEN_ID)
  {
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
    parseFactorc();
  }
  else if (isNumber)
  {
    match(makeToken(look_ahead->type, look_ahead->start, look_ahead->length, -1));
  }
  else if (look_ahead->type == TOKEN_LEFT_PAREN)
  {
    match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1));

    parseExpr();

    if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
    {
      syntaxError("Expected ')'");

      if (
          look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_RIGHT_PAREN ||
          look_ahead->type == TOKEN_COMMA ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0) ||
          look_ahead->type == TOKEN_ADD ||
          look_ahead->type == TOKEN_SUB ||
          look_ahead->type == TOKEN_MUL ||
          look_ahead->type == TOKEN_DIV ||
          look_ahead->type == TOKEN_MOD ||
          look_ahead->type == TOKEN_LT ||
          look_ahead->type == TOKEN_GT ||
          look_ahead->type == TOKEN_EQ ||
          look_ahead->type == TOKEN_LE ||
          look_ahead->type == TOKEN_GE ||
          look_ahead->type == TOKEN_NE ||
          look_ahead->type == TOKEN_RIGHT_SQUARE_PAREN)
      {
        return;
      }

      advanceToken();
      parseFactor();
    }
  }
  else
  {
    syntaxError("Expected an identifier, number, or '(' to start an expression");

    if (
        look_ahead->type == TOKEN_DOT ||
        look_ahead->type == TOKEN_SEMICOLON ||
        look_ahead->type == TOKEN_RIGHT_PAREN ||
        look_ahead->type == TOKEN_COMMA ||
        look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                              stringCompare(look_ahead->start, "od", 2) == 0 ||
                                              stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                              stringCompare(look_ahead->start, "else", 4) == 0) ||
        look_ahead->type == TOKEN_ADD ||
        look_ahead->type == TOKEN_SUB ||
        look_ahead->type == TOKEN_MUL ||
        look_ahead->type == TOKEN_DIV ||
        look_ahead->type == TOKEN_MOD ||
        look_ahead->type == TOKEN_LT ||
        look_ahead->type == TOKEN_GT ||
        look_ahead->type == TOKEN_EQ ||
        look_ahead->type == TOKEN_LE ||
        look_ahead->type == TOKEN_GE ||
        look_ahead->type == TOKEN_NE ||
        look_ahead->type == TOKEN_RIGHT_SQUARE_PAREN)
    {
      return;
    }

    advanceToken();
    parseFactor();
  }
}

void parseFactorc()
{
  // FACTORC → VARC
  // FACTORC → ( EXPRS )
  preParse("factorc");

  if (look_ahead->type == TOKEN_LEFT_PAREN)
  {
    match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1));
    parseExprs();

    if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
    {
      syntaxError("Expected closing parenthesis ')'");

      if (
          look_ahead->type == TOKEN_DOT ||
          look_ahead->type == TOKEN_SEMICOLON ||
          look_ahead->type == TOKEN_RIGHT_PAREN ||
          look_ahead->type == TOKEN_COMMA ||
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "fed", 3) == 0 ||
                                                stringCompare(look_ahead->start, "od", 2) == 0 ||
                                                stringCompare(look_ahead->start, "fi", 2) == 0 ||
                                                stringCompare(look_ahead->start, "else", 4) == 0) ||
          look_ahead->type == TOKEN_ADD ||
          look_ahead->type == TOKEN_SUB ||
          look_ahead->type == TOKEN_MUL ||
          look_ahead->type == TOKEN_DIV ||
          look_ahead->type == TOKEN_MOD ||
          look_ahead->type == TOKEN_LT ||
          look_ahead->type == TOKEN_GT ||
          look_ahead->type == TOKEN_EQ ||
          look_ahead->type == TOKEN_LE ||
          look_ahead->type == TOKEN_GE ||
          look_ahead->type == TOKEN_NE ||
          look_ahead->type == TOKEN_RIGHT_SQUARE_PAREN)
      {
        return;
      }

      advanceToken();
      parseFactorc();
    }
  }
  else
  {
    parseVarc();
  }
}

void parseExprs()
{
  // EXPRS → EXPR EXPRSC
  // EXPRS → ε
  preParse("exprs");

  if (
      look_ahead->type == TOKEN_ID ||
      look_ahead->type == TOKEN_INT ||
      look_ahead->type == TOKEN_DOUBLE ||
      look_ahead->type == TOKEN_LEFT_PAREN)
  {
    parseExpr();
    parseExprsc();
  }
  else
  {
    return;
  }
}

void parseExprsc()
{
  // EXPRSC → , EXPRS
  // EXPRSC → ε
  preParse("exprsc");

  if (look_ahead->type == TOKEN_COMMA)
  {
    match(makeToken(TOKEN_COMMA, ",", 1, -1));
    parseExprs();
  }
  else
  {
    return;
  }
}

void parseBexpr()
{
  // BEXPR → BTERM BEXPRC
  preParse("bexpr");

  parseBterm();
  parseBexprc();
}

void parseBexprc()
{
  // BEXPRC → or BTERM BEXPRC
  // BEXPRC → ε
  preParse("bexprc");

  bool isKeywordOr = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "or", 2) == 0;

  if (isKeywordOr)
  {
    match(makeToken(TOKEN_KEYWORD, "or", 2, -1));
    parseBterm();
    parseBexprc();
  }
  else
  {
    return;
  }
}

void parseBterm()
{
  // BTERM → BFACTOR BTERMC
  preParse("bterm");

  parseBfactor();
  parseBtermc();
}

void parseBtermc()
{
  // BTERMC → and BFACTOR BTERMC
  // BTERMC → ε
  preParse("btermc");

  bool isKeywordAnd = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "and", 3) == 0;

  if (isKeywordAnd)
  {
    match(makeToken(TOKEN_KEYWORD, "and", 3, -1));
    parseBfactor();
    parseBtermc();
  }
  else
  {
    return;
  }
}

// !remove
// void syncBfactor()
// {
//   while (!isAtEnd())
//   {
//     if (
//         look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "then", 4) == 0 ||
//                                               stringCompare(look_ahead->start, "do", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "or", 2) == 0 ||
//                                               stringCompare(look_ahead->start, "and", 3) == 0))
//     {
//       return;
//     }

//     advanceToken();
//   }
// }

void parseBfactor()
{
  // BFACTOR → not BFACTOR BFACTORC
  // BFACTOR → ( EXPR COMP EXPR ) BFACTORC
  preParse("bfactor");

  bool isKeywordNot = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "not", 3) == 0;
  bool isLeftParen = look_ahead->type == TOKEN_LEFT_PAREN;

  if (isKeywordNot)
  {
    match(makeToken(TOKEN_KEYWORD, "not", 3, -1));
    parseBfactor();
    parseBfactorc();
  }
  else if (isLeftParen)
  {
    match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1));
    parseExpr();
    parseComp();
    parseExpr();

    if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
    {
      syntaxError("Expected closing parenthesis ')'");

      if (
          look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "then", 4) == 0 ||
                                                stringCompare(look_ahead->start, "do", 2) == 0 ||
                                                stringCompare(look_ahead->start, "or", 2) == 0 ||
                                                stringCompare(look_ahead->start, "and", 3) == 0))
      {
        return;
      }

      advanceToken();
      parseBfactor();
    }

    parseBfactorc();
  }
  else
  {
    syntaxError("Expected 'not' or '(' for boolean factor");

    if (
        look_ahead->type == TOKEN_KEYWORD && (stringCompare(look_ahead->start, "then", 4) == 0 ||
                                              stringCompare(look_ahead->start, "do", 2) == 0 ||
                                              stringCompare(look_ahead->start, "or", 2) == 0 ||
                                              stringCompare(look_ahead->start, "and", 3) == 0))
    {
      return;
    }

    advanceToken();
    parseBfactor();
  }
}

void parseBfactorc()
{
  // BFACTORC → BTERMC BEXPRC BFACTORC
  // BFACTORC → ε
  preParse("bfactorc");

  bool isKeywordAnd = look_ahead->type == TOKEN_KEYWORD && stringCompare(look_ahead->start, "and", 3) == 0;

  if (isKeywordAnd)
  {
    parseBtermc();
    parseBexprc();
    parseBfactorc();
  }
  else
  {
    return;
  }
}

void parseComp()
{
  // COMP → <
  // COMP → >
  // COMP → ==
  // COMP → <=
  // COMP → >=
  // COMP → <>
  preParse("comp");

  switch (look_ahead->type)
  {
  case TOKEN_LT:
    match(makeToken(TOKEN_LT, "<", 1, -1));
    break;
  case TOKEN_GT:
    match(makeToken(TOKEN_GT, ">", 1, -1));
    break;
  case TOKEN_EQ:
    match(makeToken(TOKEN_EQ, "==", 2, -1));
    break;
  case TOKEN_LE:
    match(makeToken(TOKEN_LE, "<=", 2, -1));
    break;
  case TOKEN_GE:
    match(makeToken(TOKEN_GT, ">=", 2, -1));
    break;
  case TOKEN_NE:
    match(makeToken(TOKEN_NE, "<>", 2, -1));
    break;
  default:
    syntaxError("Expected a comparison operator");

    if (look_ahead->type == TOKEN_LEFT_PAREN ||
        look_ahead->type == TOKEN_ID ||
        look_ahead->type == TOKEN_INT ||
        look_ahead->type == TOKEN_DOUBLE)
    {
      return;
    }

    advanceToken();
    parseComp();
  }
}

void parseVar()
{
  // VAR → ID VARC
  preParse("var");

  if (look_ahead->type == TOKEN_ID)
  {
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
    parseVarc();
  }
  else
  {
    syntaxError("Expected an identifier");

    // if match last set, return to parent
    if (
        look_ahead->type == TOKEN_SEMICOLON ||
        look_ahead->type == TOKEN_RIGHT_PAREN ||
        look_ahead->type == TOKEN_COMMA ||
        look_ahead->type == TOKEN_ASSIGN_OP)
    {
      return;
    }

    // otherwise, move to next token and redo the current parse function
    advanceToken();
    parseVar();
  }
}

void parseVarc()
{
  // VARC → [ EXPR ]
  // VARC → ε

  preParse("varc");

  if (look_ahead->type == TOKEN_LEFT_SQUARE_PAREN)
  {
    match(makeToken(TOKEN_LEFT_SQUARE_PAREN, "[", 1, -1));
    parseExpr();
    match(makeToken(TOKEN_LEFT_SQUARE_PAREN, "]", 1, -1));
  }
  else
  {
    return;
  }
}

//< Parse Functions