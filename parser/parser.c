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
#include <stdlib.h> // For free()

#include "parser.h"
#include "../common/string.h"
#include "../common/file.h"

// todo: update 1024 to BUFFER_SIZE
#define BUFFER_SIZE 1024

// Global variables
Token *look_ahead = NULL;
Token identifiers[1024];
int identifierCount = 0;

// Variable for this module
char syntaxErrorBuffer[BUFFER_SIZE + 1];
size_t syntaxErrorBufferIndex = 0;

char symbolTableBuffer[BUFFER_SIZE + 1];
size_t symbolTableBufferIndex = 0;

// Semantic analysis global variables
// Maybe for varlist only?
DataType tempDeclarationReturnType = INT; // For declaration
DataType tempArgTypeList[10];             // For arguments
SymbolTableEntry tempArgList[10];         // For arguments, to include in function scopes
int argCount = 0;
// argument list

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

// print and create syntax error file
void syntaxError(const char *expectedMessage)
{
  char *lexeme = getTokenLexeme(look_ahead);
  char syntaxErrorMessage[BUFFER_SIZE + 1];

  sprintf(syntaxErrorMessage, "Syntax Error: %s, but found '%s' at line %d\n", expectedMessage, lexeme, look_ahead->line);

  appendToBuffer(syntaxErrorBuffer, &syntaxErrorBufferIndex, syntaxErrorMessage);
  flushBufferToFile("syntax_analysis_errors.txt", syntaxErrorBuffer, &syntaxErrorBufferIndex);
  free(lexeme);
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

  // Trigger synchronize function here
  while (!isAtEnd())
  {
    if (isInFollowSet())
    {
      return;
    }

    advanceToken();
  }
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
  syntaxErrorBuffer[BUFFER_SIZE] = '\0';
  symbolTableBuffer[BUFFER_SIZE] = '\0';
  // Add extra $ token to indicate end of input tokens
  addEndToken(tokens, &tokenCount);
  // Initialize the look ahead variable
  look_ahead = tokens;

  // Before start parsing, create a global scope
  printf("Initial scope count: %d\n", scopeCount); // Debug
  pushScope("global");

  // Start Parsing, with parseProg as the starting function in Parse()
  parseProg();
  if (look_ahead->type == TOKEN_DOLLAR)
  {
    puts("=====================");
    puts("Parsing Reach To End!");
    puts("=====================");

    // Reaching the end of the program, pop the final global scope
    popScope();
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
      return;
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
    return;
  }

  // Initialize the entry
  SymbolTableEntry entry;
  entry.symbolType = FUNCTION;
  entry.lineNumber = look_ahead->line;

  // Todo: get the type
  DataType type = parseType();
  entry.returnType = type;

  // Todo: get the function name
  char *funcName = parseFname();
  if (funcName)
  {
    _strncpy(entry.lexeme, funcName, _strlen(funcName) + 1);
    free(funcName);
  }

  if (!match(makeToken(TOKEN_LEFT_PAREN, "(", 1, -1)))
  {
    handleParseError("Expected '(' after function name", isInFollowSetForFn, parseFn);
    return;
  }

  // Todo: get arguments count and each argument type
  // How: define the arguments count global and argument type array
  parseParams();
  entry.parameterCount = argCount; // this method is simpler
  argCount = 0;                    // reset arg count;

  for (int i = 0; i < entry.parameterCount; i++)
  {
    entry.parameters[i] = tempArgTypeList[i];
  }

  // need to update the argument type

  if (!match(makeToken(TOKEN_RIGHT_PAREN, ")", 1, -1)))
  {
    handleParseError("Expected ')' after function parameters", isInFollowSetForFn, parseFn);
    return;
  }

  // Todo: insert function symbol at global here
  insertSymbol(entry);

  // Todo: add function scope here, after closing bracket for function declaration
  // Todo: get the name of the function
  pushScope("function");

  // I should add the arguments within function scopes
  for (int i = 0; i < entry.parameterCount; i++)
  {
    insertSymbol(tempArgList[i]);
  }

  parseDecls();
  parseStmts();

  if (!match(makeToken(TOKEN_KEYWORD, "fed", 3, -1)))
  {
    handleParseError("Expected 'fed' at the end of function definition", isInFollowSetForFn, parseFn);
    return;
  }

  // Todo: remove function scope here
  popScope();
}

void parseParams()
{
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  preParse("params");

  if (isKeyword("int", 3) || isKeyword("double", 6))
  {
    SymbolTableEntry entry;
    entry.parameterCount = 0;
    entry.symbolType = VARIABLE;
    entry.lineNumber = look_ahead->line;

    DataType type = parseType();
    entry.returnType = type;

    char *paramName = parseVar();
    if (paramName)
    {
      _strncpy(entry.lexeme, paramName, _strlen(paramName) + 1);
      free(paramName);
    }

    // Todo: insert parameters to function scope
    puts("params: print entry");
    printEntry(entry);
    tempArgList[argCount] = entry;
    tempArgTypeList[argCount] = type;
    argCount++;

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
      return;
    }

    SymbolTableEntry entry;
    entry.parameterCount = 0;
    entry.symbolType = VARIABLE;
    entry.lineNumber = look_ahead->line;

    DataType type = parseType();
    entry.returnType = type;

    char *paramName = parseVar();
    if (paramName)
    {
      _strncpy(entry.lexeme, paramName, _strlen(paramName) + 1);
      free(paramName);
    }

    // Todo: insert symbol here
    tempArgList[argCount] = entry;
    tempArgTypeList[argCount] = type;
    argCount++;

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

void parseFnameWrapper()
{
  // This wrapper is only used for error recovery
  // The actual type is captured via the return value of parseFname
  parseFname();
}

char *parseFname()
{
  // FNAME → ID
  // I might need the id as well
  preParse("fname");

  if (look_ahead->type == TOKEN_ID)
  {
    char *lexeme = getTokenLexeme(look_ahead);
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
    return lexeme;
  }
  else
  {
    handleParseError("Expected function name (identifier)", isInFollowSetForFname, parseFnameWrapper);
    return NULL;
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
      return;
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

// DECL -> TYPE VAR VARSC
// VARSC -> , VAR VARSC | epsilon

void parseDecl()
{
  // DECL → TYPE VARS
  preParse("decl");

  if (isKeyword("int", 3) || isKeyword("double", 6))
  {
    tempDeclarationReturnType = parseType(); // will be use for defining the variable list
    parseVars();
  }
  else
  {
    handleParseError("Expected 'int' or 'double' for declaration", isInFollowSetForDecl, parseDecl);
    return;
  }
}

bool isInFollowSetForType()
{
  return look_ahead->type == TOKEN_ID;
}

void parseTypeWrapper()
{
  // This wrapper is only used for error recovery
  // The actual type is captured via the return value of parseType
  parseType();
}

int parseType()
{
  // TYPE → int
  // TYPE → double
  preParse("type");

  if (isKeyword("int", 3))
  {
    match(makeToken(TOKEN_KEYWORD, "int", 3, -1));
    return INT;
  }
  else if (isKeyword("double", 6))
  {
    match(makeToken(TOKEN_KEYWORD, "double", 6, -1));
    return DOUBLE;
  }
  else
  {
    handleParseError("Expected 'int' or 'double' as type", isInFollowSetForType, parseTypeWrapper);
    return -1;
  }
}

void parseVars()
{
  // VARS → VAR VARSC
  preParse("vars");

  SymbolTableEntry entry;
  entry.parameterCount = 0;
  entry.symbolType = VARIABLE;
  entry.returnType = tempDeclarationReturnType;
  entry.lineNumber = look_ahead->line;

  char *varName = parseVar();
  if (varName)
  {
    _strncpy(entry.lexeme, varName, _strlen(varName) + 1);
    free(varName);
  }

  insertSymbol(entry);

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
      return;
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
      return;
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
      return;
    }

    parseStmts();

    if (!match(makeToken(TOKEN_KEYWORD, "od", 2, -1)))
    {
      handleParseError("Expected 'od' at the end of while loop", isInFollowSetForStmt, parseStmt);
      return;
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
      return;
    }
  }
  else
  {
    handleParseError("Expected 'fi' or 'else' for the end of statement", isInFollowSetForStmt, parseStmtc);
    return;
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
      return;
    }
  }
  else
  {
    handleParseError("Expected an identifier, number, or '(' to start an expression", isInFollowSetForFactor, parseFactor);
    return;
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
      return;
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
      return;
    }

    parseBfactorc();
  }
  else
  {
    handleParseError("Expected 'not' or '(' for boolean factor", isInFollowSetForBfactor, parseBfactor);
    return;
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
    return;
  }
}

bool isInFollowSetForVar()
{
  return look_ahead->type == TOKEN_SEMICOLON ||
         look_ahead->type == TOKEN_RIGHT_PAREN ||
         look_ahead->type == TOKEN_COMMA ||
         look_ahead->type == TOKEN_ASSIGN_OP;
}

void parseVarWrapper()
{
  // This wrapper is only used for error recovery
  // The actual type is captured via the return value of parseVar
  parseVar();
}

char *parseVar()
{
  // VAR → ID VARC
  preParse("var");

  if (look_ahead->type == TOKEN_ID)
  {
    char *lexeme = getTokenLexeme(look_ahead);
    match(makeToken(TOKEN_ID, look_ahead->start, look_ahead->length, -1));
    parseVarc();
    return lexeme;
  }
  else
  {
    handleParseError("Expected an identifier", isInFollowSetForVar, parseVarWrapper);
    return NULL;
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