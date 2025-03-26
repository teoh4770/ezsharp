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
#include "../common/string.h"
#include "../common/file.h"

// todo: update 1024 to BUFFER_SIZE

// Global variables
Token *look_ahead = NULL;
Token identifiers[1024];
int identifierCount = 0;

// Variable for this module
char syntaxErrorBuffer[1024 + 1];
size_t syntaxErrorBufferIndex = 0;

char symbolTableBuffer[1024 + 1];
size_t symbolTableBufferIndex = 0;

//> Helper Functions
void addEndToken(Token *tokens, int *tokenCount)
{
  // need to update the token
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

  char symbolTable[1024 + 1];
  symbolTable[1024] = '\0';

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

  char syntaxErrorMessage[1024 + 1];
  syntaxErrorMessage[1024] = '\0';
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

  // Compare token type
  if (current.type != target.type)
  {
    puts("-> Token Not Match\n");
    return false;
  }

  // Compare lexeme
  if (_strncmp(current.start, target.start, current.length) != 0)
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

bool isKeyword(const char *keyword, int length)
{
  return look_ahead->type == TOKEN_KEYWORD && _strncmp(look_ahead->start, keyword, length) == 0;
}

// Handle parse error
// Accept both a follow set function
// And a callback for re-parsing
void handleParseError(const char *message, bool (*isInFollowSet)(), void (*parseFunc)())
{
  syntaxError(message);

  if (isAtEnd() || isInFollowSet())
  {
    return;
  }

  advanceToken();
  parseFunc();
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
  syntaxErrorBuffer[1024] = '\0';
  symbolTableBuffer[1024] = '\0';
  // Add extra $ token to indicate end of input tokens
  addEndToken(tokens, &tokenCount);
  // Initialize the look ahead variable
  look_ahead = tokens;

  // Before start parsing, create a global scope
  printf("Initial scope count: %d\n", scopeCount);        // Debug
  printf("Initial look_ahead: %p\n", (void *)look_ahead); // Debug

  // This is troublesome!
  // pushScope("global");

  // Start Parsing, with parseProg as the starting function in Parse()
  parseProg();
  if (look_ahead->type == TOKEN_DOLLAR)
  {
    puts("=====================");
    puts("Parsing Reach To End!");
    puts("=====================");

    printSymbolTable();

    // Reaching the end of the program, pop the final global scope
    // popScope();
  }
  else
  {
    puts("=========================");
    puts("Parsing Not Reach To End!");
    puts("=========================");
  }
}

void syncProg()
{
  while (!isAtEnd())
  {
    advanceToken();
  }
}

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
    syncProg();
    return;
  }
}

bool isInFollowSetForFns()
{
  switch (look_ahead->type)
  {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
  case TOKEN_ID:
  case TOKEN_INT:
  case TOKEN_DOUBLE:
    return true;
  case TOKEN_KEYWORD:
    return isKeyword("if", 2) ||
           isKeyword("while", 5) ||
           isKeyword("print", 5) ||
           isKeyword("return", 6);
  default:
    return false;
  }
}

void parseFns()
{
  // FNS → FN ; FNSC
  // FNS → ε
  preParse("fns");

  if (isKeyword("def", 3))
  {
    parseFn();

    if (!match(makeToken(TOKEN_SEMICOLON, ";", 1, -1)))
    {
      handleParseError("Expected ';' after function definition", isInFollowSetForFns, parseFns);
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

bool isInFollowSetForFn()
{
  return look_ahead->type == TOKEN_SEMICOLON;
}

void parseFn()
{
  // FN → def TYPE FNAME ( PARAMS ) DECLS STMTS fed
  preParse("fn");

  if (!match(makeToken(TOKEN_KEYWORD, "def", 3, -1)))
  {
    handleParseError("Expected 'def' at the start of function definition", isInFollowSetForFn, parseFn);
  }

  // Todo: get the type
  parseType();
  // Todo: get the function name
  parseFname();

  if (!match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1)))
  {
    handleParseError("Expected '(' after function name", isInFollowSetForFn, parseFn);
  }

  // Todo: get arguments count and each argument type
  parseParams();

  if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
  {
    handleParseError("Expected ')' after function parameters", isInFollowSetForFn, parseFn);
  }

  // Todo: insert function symbol at global here
  // ?need to get line number, lexeme, return type, argument counts and "each argument type"

  // Todo: add function scope here, after closing bracket for function declaration
  // Todo: get the name of the function
  // pushScope("function");

  parseDecls();
  parseStmts();

  if (!match(makeToken(TOKEN_KEYWORD, "fed", 3, -1)))
  {
    handleParseError("Expected 'fed' at the end of function definition", isInFollowSetForFn, parseFn);
  }

  // Todo: remove function scope here
  // popScope();
}

void parseParams()
{
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  preParse("params");

  if (isKeyword("int", 3) || isKeyword("double", 6))
  {
    parseType();
    parseVar();
    // Todo: insert parameters here
    parseParamsc();
  }
  else
  {
    return;
  }
}

bool isInFollowSetForParamsc()
{
  return look_ahead->type == TOKEN_RIGHT_PAREN;
}

void parseParamsc()
{
  // PARAMSC → , TYPE VAR PARAMSC | ε
  preParse("paramsc");

  if (look_ahead->type == TOKEN_COMMA)
  {
    match(makeToken(TOKEN_COMMA, ",", 1, -1));

    if (!(isKeyword("int", 3) || isKeyword("double", 6)))
    {
      handleParseError("Expected a type ('int' or 'double') after ',' in parameter list", isInFollowSetForParamsc, parseParamsc);
    }

    parseType();
    parseVar();
    // Todo: insert symbol here
    parseParamsc();
  }
  else
  {
    return;
  }
}

bool isInFollowSetForFname()
{
  return look_ahead->type == TOKEN_LEFT_PAREN;
}

void parseFname()
{
  // FNAME → ID
  // I might need the id as well
  preParse("fname");

  if (look_ahead->type == TOKEN_ID)
  {
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
  }
  else
  {
    handleParseError("Expected function name (identifier)", isInFollowSetForFname, parseFname);
  }
}

bool isInFollowSetForDecls()
{
  switch (look_ahead->type)
  {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
  case TOKEN_ID:
    return true;
  case TOKEN_KEYWORD:
    return isKeyword("fed", 3) ||
           isKeyword("if", 2) ||
           isKeyword("while", 5) ||
           isKeyword("print", 5) ||
           isKeyword("return", 6);
  default:
    return false;
  }
}

void parseDecls()
{
  // DECLS → DECL; DECLSC
  // DECLS → ε
  preParse("decls");

  if (isKeyword("int", 3) || isKeyword("double", 6))
  {
    parseDecl();

    if (!match(makeToken(TOKEN_SEMICOLON, ";", 1, -1)))
    {
      handleParseError("Expected semicolon", isInFollowSetForDecls, parseDecls);
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

bool isInFollowSetForDecl()
{
  return look_ahead->type == TOKEN_SEMICOLON;
}

void parseDecl()
{
  // DECL → TYPE VARS
  preParse("decl");

  if (isKeyword("int", 3) || isKeyword("double", 6))
  {
    parseType();
    parseVars();
    // Todo: insert parameters here
  }
  else
  {
    handleParseError("Expected 'int' or 'double' for declaration", isInFollowSetForDecl, parseDecl);
  }
}

bool isInFollowSetForType()
{
  return look_ahead->type == TOKEN_ID;
}

void parseType()
{
  // TYPE → int
  // TYPE → double
  preParse("type");

  if (isKeyword("int", 3))
  {
    match(makeToken(TOKEN_KEYWORD, "int", 3, -1));
  }
  else if (isKeyword("double", 6))
  {
    match(makeToken(TOKEN_KEYWORD, "double", 6, -1));
  }
  else
  {
    handleParseError("Expected 'int' or 'double' as type", isInFollowSetForType, parseType);
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

bool isInFollowSetForStmt()
{
  switch (look_ahead->type)
  {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
    return true;

  case TOKEN_KEYWORD:
    return isKeyword("fed", 3) ||
           isKeyword("fi", 2) ||
           isKeyword("od", 2) ||
           isKeyword("else", 4);

  default:
    return false;
  }
}

void parseStmt()
{
  // STMT → VAR = EXPR
  // STMT → if BEXPR then STMTS STMTC
  // STMT → while BEXPR do STMTS od
  // STMT → print EXPR
  // STMT -> return EXPR
  // STMT →  ε
  preParse("stmt");

  if (look_ahead->type == TOKEN_ID)
  {
    parseVar();

    if (!match(makeToken(TOKEN_ASSIGN_OP, "=", 1, -1)))
    {
      handleParseError("Expected '=' for assignment", isInFollowSetForStmt, parseStmt);
    }

    parseExpr();
  }
  else if (isKeyword("if", 2))
  {
    match(makeToken(TOKEN_KEYWORD, "if", 2, -1));
    parseBexpr();

    if (!match(makeToken(TOKEN_KEYWORD, "then", 4, -1)))
    {
      handleParseError("Missing 'then' after 'if' statement", isInFollowSetForStmt, parseStmt);
    }

    parseStmts();
    parseStmtc();
  }
  else if (isKeyword("while", 5))
  {
    match(makeToken(TOKEN_KEYWORD, "while", 5, -1));
    parseBexpr();

    if (!match(makeToken(TOKEN_KEYWORD, "do", 2, -1)))
    {
      handleParseError("Missing 'do' after 'while' statement", isInFollowSetForStmt, parseStmt);
    }

    parseStmts();

    if (!match(makeToken(TOKEN_KEYWORD, "od", 2, -1)))
    {
      handleParseError("Expected 'od' at the end of while loop", isInFollowSetForStmt, parseStmt);
    }
  }
  else if (isKeyword("print", 5))
  {
    match(makeToken(TOKEN_KEYWORD, "print", 5, -1));
    parseExpr();
  }
  else if (isKeyword("return", 6))
  {
    match(makeToken(TOKEN_KEYWORD, "return", 6, -1));
    parseExpr();
  }
  else
  {
    return;
  }
}

void parseStmtc()
{
  // STMTC → fi
  // STMTC → else STMTS fi
  preParse("stmtc");

  if (isKeyword("fi", 2))
  {
    match(makeToken(TOKEN_KEYWORD, "fi", 2, -1));
  }
  else if (isKeyword("else", 4))
  {
    match(makeToken(TOKEN_KEYWORD, "else", 4, -1));

    parseStmts();

    if (!match(makeToken(TOKEN_KEYWORD, "fi", 2, -1)))
    {
      handleParseError("Statement does not end with 'fi'", isInFollowSetForStmt, parseStmtc);
    }
  }
  else
  {
    handleParseError("Expected 'fi' or 'else' for the end of statement", isInFollowSetForStmt, parseStmtc);
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

bool isInFollowSetForFactor()
{
  switch (look_ahead->type)
  {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
  case TOKEN_RIGHT_PAREN:
  case TOKEN_COMMA:
  case TOKEN_ADD:
  case TOKEN_SUB:
  case TOKEN_MUL:
  case TOKEN_DIV:
  case TOKEN_MOD:
  case TOKEN_LT:
  case TOKEN_GT:
  case TOKEN_EQ:
  case TOKEN_LE:
  case TOKEN_GE:
  case TOKEN_NE:
  case TOKEN_RIGHT_SQUARE_PAREN:
    return true;

  case TOKEN_KEYWORD:
    return isKeyword("fed", 3) ||
           isKeyword("fi", 2) ||
           isKeyword("od", 2) ||
           isKeyword("else", 4);

  default:
    return false;
  }
}

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
      handleParseError("Expected ')'", isInFollowSetForFactor, parseFactor);
    }
  }
  else
  {
    handleParseError("Expected an identifier, number, or '(' to start an expression", isInFollowSetForFactor, parseFactor);
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
      handleParseError("Expected closing parenthesis ')'", isInFollowSetForFactor, parseFactorc);
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

  if (isKeyword("or", 2))
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

  if (isKeyword("and", 3))
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

bool isInFollowSetForBfactor()
{
  return isKeyword("then", 4) ||
         isKeyword("do", 2) ||
         isKeyword("or", 2) ||
         isKeyword("and", 3);
}

void parseBfactor()
{
  // BFACTOR → not BFACTOR BFACTORC
  // BFACTOR → ( EXPR COMP EXPR ) BFACTORC
  preParse("bfactor");

  if (isKeyword("not", 3))
  {
    match(makeToken(TOKEN_KEYWORD, "not", 3, -1));
    parseBfactor();
    parseBfactorc();
  }
  else if (look_ahead->type == TOKEN_LEFT_PAREN)
  {
    match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1));
    parseExpr();
    parseComp();
    parseExpr();

    if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
    {
      handleParseError("Expected closing parenthesis ')'", isInFollowSetForBfactor, parseBfactor);
    }

    parseBfactorc();
  }
  else
  {
    handleParseError("Expected 'not' or '(' for boolean factor", isInFollowSetForBfactor, parseBfactor);
  }
}

void parseBfactorc()
{
  // BFACTORC → BTERMC BEXPRC BFACTORC
  // BFACTORC → ε
  preParse("bfactorc");

  if (isKeyword("and", 3))
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

bool isInFollowSetForComp()
{
  return look_ahead->type == TOKEN_LEFT_PAREN ||
         look_ahead->type == TOKEN_ID ||
         look_ahead->type == TOKEN_INT ||
         look_ahead->type == TOKEN_DOUBLE;
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
    handleParseError("Expected a comparison operator", isInFollowSetForComp, parseComp);
  }
}

bool isInFollowSetForVar()
{
  return look_ahead->type == TOKEN_SEMICOLON ||
         look_ahead->type == TOKEN_RIGHT_PAREN ||
         look_ahead->type == TOKEN_COMMA ||
         look_ahead->type == TOKEN_ASSIGN_OP;
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
    handleParseError("Expected an identifier", isInFollowSetForVar, parseVar);
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