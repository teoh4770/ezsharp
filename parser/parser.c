// parser.c: the main component to handle parsing

#include <fcntl.h> // For open() flags
#include <stdio.h>
#include <stdlib.h> // For free()
#include <unistd.h> // For write() and close()

#include "../common/file.h"
#include "../common/string.h"
#include "parser.h"

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
// For varlist only
DataType tempDeclarationReturnType = INT;
DataType tempArgTypeList[10];
SymbolTableEntry tempArgList[10];
int argCount = 0;

//> Helper Functions
void addEndToken(Token *tokens, int *tokenCount) {
  // need to update the token
  tokens[*tokenCount] = makeToken(TOKEN_DOLLAR, "$", 1, -1);
  (*tokenCount)++;
}

void preParse(const char *message) {
  printf("Currently parsing: %s\n", message);
}

void advanceToken() {
  if (look_ahead->type != TOKEN_DOLLAR) {
    look_ahead++;
  }
}

Token *previousToken() { return look_ahead - 1; }

Token *nextToken() { return look_ahead + 1; }

void syntaxError(const char *expectedMessage) {
  char *lexeme = getTokenLexeme(look_ahead);
  char syntaxErrorMessage[BUFFER_SIZE + 1];

  // Print and create syntax error file
  sprintf(syntaxErrorMessage, "Syntax Error: %s, but found '%s' at line %d\n",
          expectedMessage, lexeme, look_ahead->line);

  appendToBuffer(syntaxErrorBuffer, &syntaxErrorBufferIndex, syntaxErrorMessage,
                 "syntax_analysis_errors.txt");
  flushBufferToFile("syntax_analysis_errors.txt", syntaxErrorBuffer,
                    &syntaxErrorBufferIndex);

  free(lexeme);
}

bool matchToken(Token current, Token target) {
  // Compare token type and lexeme
  if (current.type != target.type ||
      _strncmp(current.lexeme, target.lexeme, current.length) != 0) {
    return false;
  }

  return true;
};

bool isAtEnd() { return look_ahead->type == TOKEN_DOLLAR; }

bool isKeyword(const char *keyword, int length) {
  return look_ahead->type == TOKEN_KEYWORD &&
         _strncmp(look_ahead->lexeme, keyword, length) == 0;
}

bool isComparison(TokenType type) {
  return type == TOKEN_LT || type == TOKEN_LE || type == TOKEN_GT ||
         type == TOKEN_GE || type == TOKEN_EQ || type == TOKEN_NE;
}

bool isNumber(TokenType type) {
  return type == TOKEN_INT || type == TOKEN_DOUBLE;
}

void handleParseError(const char *message, bool (*isInFollowSet)()) {
  syntaxError(message);

  while (!isAtEnd()) {
    if (isInFollowSet()) {
      return;
    }

    advanceToken();
  }
}

// Push scope operation
void A(char *scopeName) { pushScope(scopeName); }

// Pop scope operation
void B() { popScope(); }

// Insert symbol operation
void C(SymbolType symbolType, DataType returnType, int lineNumber,
       int parameterCount, char *symbolName) {
  SymbolTableEntry entry;
  entry.symbolType = symbolType;
  entry.returnType = returnType;
  entry.lineNumber = lineNumber;
  entry.parameterCount = parameterCount;

  // Update entry name
  _strncpy(entry.lexeme, symbolName, _strlen(symbolName) + 1);

  // Update argument type list
  for (int i = 0; i < parameterCount; i++) {
    entry.parameters[i] = tempArgTypeList[i];
  }

  insertSymbol(entry);
}

// Look up symbol when a variable is being reference
SymbolTableEntry *D(const char *lexeme) { return lookupSymbol(lexeme); }

void resetArgCount() { argCount = 0; }

//< Helper functions

//> Parse Functions
bool match(TokenType tokenType, char *lexeme) {
  // Line number is set to -1, since it is not important for token match
  Token targetToken = makeToken(tokenType, lexeme, _strlen(lexeme), -1);

  if (!matchToken(*look_ahead, targetToken)) {
    return false;
  }

  if (look_ahead->type == TOKEN_ID) {
    identifiers[identifierCount++] = *look_ahead;
  }

  advanceToken();

  return true;
}

void Parse(Token *tokens, int tokenCount) {
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

  // Start Parsing, with parseProg as the starting function
  parseProg();
  if (look_ahead->type == TOKEN_DOLLAR) {
    puts("=====================");
    puts("Parsing Reach To End!");
    puts("=====================");
  } else {
    puts("=========================");
    puts("Parsing Not Reach To End!");
    puts("=========================");
  }
}

void syncProg() {
  while (!isAtEnd()) {
    advanceToken();
  }
}

void parseProg() {
  // PROG → A FNS DECLS STMTS B .
  preParse("prog");

  A("global");
  parseFns();
  parseDecls();
  parseStmts();
  B();

  if (!match(TOKEN_DOT, ".")) {
    syntaxError("Expected '.' to indicate end of the program");
    syncProg();

    return;
  }
}

bool isInFollowSetForFns() {
  switch (look_ahead->type) {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
  case TOKEN_ID:
  case TOKEN_INT:
  case TOKEN_DOUBLE:
    return true;
  case TOKEN_KEYWORD:
    return isKeyword("if", 2) || isKeyword("while", 5) ||
           isKeyword("print", 5) || isKeyword("return", 6);
  default:
    return false;
  }
}

void parseFns() {
  // FNS → FN ; FNSC
  // FNS → ε
  preParse("fns");

  if (isKeyword("def", 3)) {
    parseFn();

    if (!match(TOKEN_SEMICOLON, ";")) {
      handleParseError("Expected ';' after function definition",
                       isInFollowSetForFns);
      return;
    }

    parseFnsc();

    return;
  }

  return;
}

void parseFnsc() {
  // FNSC → FN; FNSC
  // FNSC → ε
  preParse("fnsc");
  parseFns();
}

bool isInFollowSetForFn() { return look_ahead->type == TOKEN_SEMICOLON; }

void parseFn() {
  // FN → def TYPE FNAME ( PARAMS ) C A C DECLS STMTS fed B
  preParse("fn");

  if (!match(TOKEN_KEYWORD, "def")) {
    handleParseError("Expected 'def' at the start of function definition",
                     isInFollowSetForFn);
    return;
  }

  DataType type = parseType();
  char *funcName = parseFname();

  if (!match(TOKEN_LEFT_PAREN, "(")) {
    handleParseError("Expected '(' after function name", isInFollowSetForFn);
    return;
  }

  // Update argument counts
  parseParams();

  if (!match(TOKEN_RIGHT_PAREN, ")")) {
    handleParseError("Expected ')' after function parameters",
                     isInFollowSetForFn);
    return;
  }

  // C: Insert function symbol at global scope
  // A: Insert function scope
  // C: Insert argument symbol at function scope
  C(FUNCTION, type, look_ahead->line, argCount, funcName);

  A(funcName);

  for (int i = 0; i < argCount; i++) {
    C(tempArgList[i].symbolType, tempArgList[i].returnType,
      tempArgList[i].lineNumber, 0, (tempArgList[i].lexeme));
  }
  resetArgCount();

  parseDecls();
  parseStmts();

  if (!match(TOKEN_KEYWORD, "fed")) {
    handleParseError("Expected 'fed' at the end of function definition",
                     isInFollowSetForFn);
    return;
  }

  // Pop function scope
  B();

  free(funcName);
}

void parseParams() {
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  preParse("params");

  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    DataType type = parseType();

    char *paramName = parseVar();

    // ? Refactor
    // Create symbol table entry for upcoming argument
    SymbolTableEntry entry;
    entry.parameterCount = 0;
    entry.symbolType = VARIABLE;
    entry.lineNumber = look_ahead->line;
    entry.returnType = type;

    if (paramName) {
      _strncpy(entry.lexeme, paramName, _strlen(paramName) + 1);
      free(paramName);
    }

    // Update temporarily argument list and argument type list
    tempArgList[argCount] = entry;
    tempArgTypeList[argCount] = type;
    argCount++;

    parseParamsc();

    return;
  }

  return;
}

bool isInFollowSetForParamsc() { return look_ahead->type == TOKEN_RIGHT_PAREN; }

void parseParamsc() {
  // PARAMSC → , TYPE VAR PARAMSC | ε
  preParse("paramsc");

  if (match(TOKEN_COMMA, ",")) {
    if (!(isKeyword("int", 3) || isKeyword("double", 6))) {
      handleParseError(
          "Expected a type ('int' or 'double') after ',' in parameter list",
          isInFollowSetForParamsc);
      return;
    }

    DataType type = parseType();
    char *paramName = parseVar();

    // ? Refactor
    SymbolTableEntry entry;
    entry.parameterCount = 0;
    entry.symbolType = VARIABLE;
    entry.lineNumber = look_ahead->line;
    entry.returnType = type;

    if (paramName) {
      _strncpy(entry.lexeme, paramName, _strlen(paramName) + 1);
      free(paramName);
    }

    // Update temporarily argument list and argument type list
    tempArgList[argCount] = entry;
    tempArgTypeList[argCount] = type;
    argCount++;

    parseParamsc();

    return;
  }

  return;
}

bool isInFollowSetForFname() { return look_ahead->type == TOKEN_LEFT_PAREN; }

char *parseFname() {
  // FNAME → ID
  preParse("fname");

  if (look_ahead->type == TOKEN_ID) {
    char *lexeme = look_ahead->lexeme;
    match(TOKEN_ID, lexeme);

    return lexeme;
  }

  handleParseError("Expected function name (identifier)",
                   isInFollowSetForFname);
  return NULL;
}

bool isInFollowSetForDecls() {
  switch (look_ahead->type) {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
  case TOKEN_ID:
    return true;
  case TOKEN_KEYWORD:
    return isKeyword("fed", 3) || isKeyword("if", 2) || isKeyword("while", 5) ||
           isKeyword("print", 5) || isKeyword("return", 6);
  default:
    return false;
  }
}

void parseDecls() {
  // DECLS → DECL; DECLSC
  // DECLS → ε
  preParse("decls");

  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    parseDecl();

    if (!match(TOKEN_SEMICOLON, ";")) {
      handleParseError("Expected semicolon", isInFollowSetForDecls);
      return;
    }

    parseDeclsc();

    return;
  }

  return;
}

void parseDeclsc() {
  // DECLSC → DECL; DECLSC
  // DECLSC → ε
  preParse("declsc");
  parseDecls();
}

bool isInFollowSetForDecl() { return look_ahead->type == TOKEN_SEMICOLON; }

// DECL -> TYPE VARS

void parseDecl() {
  // DECL → TYPE VARS
  preParse("decl");

  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    // Will be use for defining the type of variable list
    tempDeclarationReturnType = parseType();
    parseVars();

    return;
  }

  handleParseError("Expected 'int' or 'double' for declaration",
                   isInFollowSetForDecl);

  return;
}

bool isInFollowSetForType() { return look_ahead->type == TOKEN_ID; }

int parseType() {
  // TYPE → int
  // TYPE → double
  preParse("type");

  if (match(TOKEN_KEYWORD, "int")) {
    return INT;
  } else if (match(TOKEN_KEYWORD, "double")) {
    return DOUBLE;
  } else {
    handleParseError("Expected 'int' or 'double' as type",
                     isInFollowSetForType);
    return -1;
  }
}

void parseVars() {
  // VARS → VAR C VARSC
  preParse("vars");

  char *variableName = parseVar();
  C(VARIABLE, tempDeclarationReturnType, look_ahead->line, 0, variableName);
  parseVarsc();

  free(variableName);
}

void parseVarsc() {
  // VARSC → , VARS
  // VARSC → ε
  preParse("varsc");

  if (match(TOKEN_COMMA, ",")) {
    parseVars();
    return;
  }

  return;
}

void parseStmts() {
  // STMTS → STMT STMTSC
  preParse("stmts");

  parseStmt();
  parseStmtsc();
}

void parseStmtsc() {
  // STMTSC → ; STMTS
  // STMTSC → ε
  preParse("stmtsc");

  if (match(TOKEN_SEMICOLON, ";")) {
    parseStmts();
    return;
  }

  return;
}

bool isInFollowSetForStmt() {
  switch (look_ahead->type) {
  case TOKEN_DOT:
  case TOKEN_SEMICOLON:
    return true;
  case TOKEN_KEYWORD:
    return isKeyword("fed", 3) || isKeyword("fi", 2) || isKeyword("od", 2) ||
           isKeyword("else", 4);
  default:
    return false;
  }
}

void parseStmt() {
  // STMT → VAR D = EXPR
  // STMT → if BEXPR then STMTS STMTC
  // STMT → while BEXPR do STMTS od
  // STMT → print EXPR
  // STMT -> return EXPR
  // STMT →  ε
  preParse("stmt");

  if (look_ahead->type == TOKEN_ID) {
    char *variableName = parseVar();

    SymbolTableEntry *variable = D(variableName);
    if (!variable) {
      printf("Undeclared variable %s at line number %d\n", variableName,
             look_ahead->line);
    }

    if (!match(TOKEN_ASSIGN_OP, "=")) {
      handleParseError("Expected '=' for assignment", isInFollowSetForStmt);
      return;
    }

    // Get the return type of expression
    DataType rightType = parseExpr();

    // Type checking: Ensure LHS (variable) type matches RHS (expression) type
    if (variable && variable->returnType != rightType) {
      printf("Semantic Error: Type mismatch in assignment at line %d. "
             "Variable '%s' is of type '%s', but assigned expression of type "
             "'%s'.\n",
             look_ahead->line, variableName,
             dataTypeToString(variable->returnType),
             dataTypeToString(rightType));
    }
  } else if (isKeyword("if", 2)) {
    match(TOKEN_KEYWORD, "if");
    parseBexpr();

    if (!match(TOKEN_KEYWORD, "then")) {
      handleParseError("Missing 'then' after 'if' statement",
                       isInFollowSetForStmt);
      return;
    }

    parseStmts();
    parseStmtc();
  } else if (match(TOKEN_KEYWORD, "while")) {
    parseBexpr();

    if (!match(TOKEN_KEYWORD, "do")) {
      handleParseError("Missing 'do' after 'while' statement",
                       isInFollowSetForStmt);
      return;
    }

    parseStmts();

    if (!match(TOKEN_KEYWORD, "od")) {
      handleParseError("Expected 'od' at the end of while loop",
                       isInFollowSetForStmt);
      return;
    }
  } else if (match(TOKEN_KEYWORD, "print")) {
    parseExpr();
  } else if (match(TOKEN_KEYWORD, "return")) {
    DataType returnType = parseExpr();

    // Compare the return value with function return type
    SymbolTableEntry *functionEntry = getFunctionEntry();

    if (returnType != functionEntry->returnType) {
      printf("Type error: Function declared as %d but returning %d\n",
             functionEntry->returnType, returnType);
    }
  } else {
    return;
  }
}

void parseStmtc() {
  // STMTC → fi
  // STMTC → else STMTS fi
  preParse("stmtc");

  if (match(TOKEN_KEYWORD, "fi")) {
    return;
  }

  if (match(TOKEN_KEYWORD, "else")) {
    parseStmts();

    if (!match(TOKEN_KEYWORD, "fi")) {
      handleParseError("Statement does not end with 'fi'",
                       isInFollowSetForStmt);
      return;
    }

    return;
  }

  handleParseError("Expected 'fi' or 'else' for the end of statement",
                   isInFollowSetForStmt);
  return;
}

DataType parseExpr() {
  // EXPR → TERM EXPRC
  preParse("expr");

  DataType leftType = parseTerm();
  return parseExprc(leftType);
}

DataType parseExprc(DataType leftType) {
  // EXPRC → + TERM EXPRC
  // EXPRC → - TERM EXPRC
  // EXPRC → ε
  preParse("exprc");

  if (look_ahead->type == TOKEN_ADD || look_ahead->type == TOKEN_SUB) {
    Token *opToken = look_ahead;
    match(look_ahead->type, look_ahead->lexeme);

    DataType rightType = parseTerm();

    if (leftType != rightType) {
      printf("Semantic Error: Type mismatch in arithmetic operation '%s' at "
             "line %d. "
             "Left operand is '%s', but right operand is '%s'.\n",
             opToken->lexeme, opToken->line, dataTypeToString(leftType),
             dataTypeToString(rightType));
    }

    return parseExprc(leftType);
  }

  return leftType;
}

DataType parseTerm() {
  // TERM → FACTOR TERMC
  preParse("term");

  DataType leftType = parseFactor();
  return parseTermc(leftType);
}

DataType parseTermc(DataType leftType) {
  // TERMC → * FACTOR TERMC
  // TERMC → / FACTOR TERMC
  // TERMC → % FACTOR TERMC
  // TERMC → ε

  preParse("termc");

  if (look_ahead->type == TOKEN_MUL || look_ahead->type == TOKEN_DIV ||
      look_ahead->type == TOKEN_MOD) {
    Token opToken = *look_ahead;
    match(look_ahead->type, look_ahead->lexeme);

    DataType rightType = parseFactor();

    if (leftType != rightType) {
      printf("Semantic Error: Type mismatch in arithmetic operation '%s' at "
             "line %d. "
             "Left operand is '%s', but right operand is '%s'.\n",
             opToken.lexeme, opToken.line, dataTypeToString(leftType),
             dataTypeToString(rightType));
      return ERROR;
    }

    return parseTermc(leftType);
  }

  return leftType;
}

bool isInFollowSetForFactor() {
  switch (look_ahead->type) {
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
    return isKeyword("fed", 3) || isKeyword("fi", 2) || isKeyword("od", 2) ||
           isKeyword("else", 4);

  default:
    return false;
  }
}

DataType parseFactor() {
  // FACTOR → ID D FACTORC
  // FACTOR → NUMBER
  // FACTOR → (EXPR)
  preParse("factor");

  if (look_ahead->type == TOKEN_ID) {
    // Look up the identifier in symbol table
    // If found, return the symbol's return type
    SymbolTableEntry *symbol = D(look_ahead->lexeme);
    if (!symbol) {
      printf("Undeclared variable %s at line number %d\n", look_ahead->lexeme,
             look_ahead->line);
      return ERROR;
    }

    match(TOKEN_ID, look_ahead->lexeme);
    parseFactorc(symbol);

    return symbol->returnType;
  }

  if (isNumber(look_ahead->type)) {
    DataType numberType = look_ahead->type == TOKEN_INT ? INT : DOUBLE;

    match(look_ahead->type, look_ahead->lexeme);

    return numberType;
  }

  if (match(TOKEN_LEFT_PAREN, "(")) {
    DataType exprType = parseExpr();

    if (exprType == ERROR) {
      puts("Semantic Error");
    }

    if (!match(TOKEN_RIGHT_PAREN, ")")) {
      handleParseError("Expected ')'", isInFollowSetForFactor);
      return ERROR;
    }

    return exprType;
  }

  handleParseError(
      "Expected an identifier, number, or '(' to start an expression",
      isInFollowSetForFactor);
  return ERROR;
}

void parseFactorc(SymbolTableEntry *symbol) {
  // FACTORC → VARC
  // FACTORC → ( EXPRS )
  preParse("factorc");

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    if (symbol->symbolType != FUNCTION) {
      printf("Semantic Error: '%s' is not a function but is used as one (line "
             "%d).\n",
             symbol->lexeme, look_ahead->line);
    }

    match(TOKEN_LEFT_PAREN, "(");

    // Parse argument expressions and track types
    parseExprs();

    // Get the function entry
    SymbolTableEntry *functionEntry = lookupSymbol(symbol->lexeme);

    // Check argument count mismatch
    if (argCount != functionEntry->parameterCount) {
      if (argCount > functionEntry->parameterCount) {
        printf("Semantic Error: Too many arguments for function '%s' (line "
               "%d). Expected %d, but got %d.\n",
               functionEntry->lexeme, look_ahead->line,
               functionEntry->parameterCount, argCount);
      } else {
        printf("Semantic Error: Too few arguments for function '%s' (line %d). "
               "Expected %d, but got %d.\n",
               functionEntry->lexeme, look_ahead->line,
               functionEntry->parameterCount, argCount);
      }
    }

    // Check argument type mismatches
    for (int i = 0; i < functionEntry->parameterCount; i++) {
      if (functionEntry->parameters[i] != tempArgTypeList[i]) {
        printf("Semantic Error: Argument %d of function '%s' (line %d) has "
               "incorrect type. Expected '%s', but got '%s'.\n",
               i + 1, functionEntry->lexeme, look_ahead->line,
               dataTypeToString(functionEntry->parameters[i]),
               dataTypeToString(tempArgTypeList[i]));
      }
    }

    resetArgCount();

    if (!match(TOKEN_RIGHT_PAREN, ")")) {
      handleParseError("Expected closing parenthesis ')'",
                       isInFollowSetForFactor);
      return;
    }

    return;
  }

  parseVarc();
}

void parseExprs() {
  // EXPRS → EXPR EXPRSC
  // EXPRS → ε
  preParse("exprs");

  if (look_ahead->type == TOKEN_ID || look_ahead->type == TOKEN_INT ||
      look_ahead->type == TOKEN_DOUBLE ||
      look_ahead->type == TOKEN_LEFT_PAREN) {
    // check for previous
    // check the argument type
    DataType type = parseExpr();
    tempArgTypeList[argCount++] = type;
    // check if it matched the arg type
    // increment temp arg count
    parseExprsc();

    return;
  }

  return;
}

void parseExprsc() {
  // EXPRSC → , EXPRS
  // EXPRSC → ε
  preParse("exprsc");

  if (match(TOKEN_COMMA, ",")) {
    parseExprs();
    return;
  }

  return;
}

void parseBexpr() {
  // BEXPR → BTERM BEXPRC
  preParse("bexpr");

  parseBterm();
  parseBexprc();
}

void parseBexprc() {
  // BEXPRC → or BTERM BEXPRC
  // BEXPRC → ε
  preParse("bexprc");

  if (match(TOKEN_KEYWORD, "or")) {
    parseBterm();
    parseBexprc();

    return;
  }

  return;
}

void parseBterm() {
  // BTERM → BFACTOR BTERMC
  preParse("bterm");

  parseBfactor();
  parseBtermc();
}

void parseBtermc() {
  // BTERMC → and BFACTOR BTERMC
  // BTERMC → ε
  preParse("btermc");

  if (match(TOKEN_KEYWORD, "and")) {
    parseBfactor();
    parseBtermc();

    return;
  }

  return;
}

bool isInFollowSetForBfactor() {
  return isKeyword("then", 4) || isKeyword("do", 2) || isKeyword("or", 2) ||
         isKeyword("and", 3);
}

void parseBfactor() {
  // BFACTOR → not bfactor
  // BFACTOR → (bexpr)
  // BFACTOR → (expr comp expr)
  preParse("bfactor");

  if (match(TOKEN_KEYWORD, "not")) {
    parseBfactor();
    return;
  }

  if (match(TOKEN_LEFT_PAREN, "(")) {
    if (isComparison(nextToken()->type)) {
      // BFACTOR → (expr comp expr)
      DataType leftType = parseExpr();
      parseComp();
      DataType rightType = parseExpr();

      if (leftType != rightType) {
        printf("Type error: Type mismatch in comparison at line %d\n",
               look_ahead->line);
      }

      if (!match(TOKEN_RIGHT_PAREN, ")")) {
        handleParseError("Expected closing parenthesis ')'",
                         isInFollowSetForBfactor);
        return;
      }
    } else {
      // BFACTOR → (bexpr)
      parseBexpr();

      if (!match(TOKEN_RIGHT_PAREN, ")")) {
        handleParseError("Expected closing parenthesis ')'",
                         isInFollowSetForBfactor);
        return;
      }
    }

    return;
  }

  handleParseError("Expected 'not' or '(' for boolean factor",
                   isInFollowSetForBfactor);
  return;
}

bool isInFollowSetForComp() {
  return look_ahead->type == TOKEN_LEFT_PAREN || look_ahead->type == TOKEN_ID ||
         look_ahead->type == TOKEN_INT || look_ahead->type == TOKEN_DOUBLE;
}

void parseComp() {
  // COMP → <
  // COMP → >
  // COMP → ==
  // COMP → <=
  // COMP → >=
  // COMP → <>
  preParse("comp");

  if (isComparison(look_ahead->type)) {
    match(look_ahead->type, look_ahead->lexeme);
    return;
  }

  handleParseError("Expected a comparison operator", isInFollowSetForComp);
  return;
}

bool isInFollowSetForVar() {
  return look_ahead->type == TOKEN_SEMICOLON ||
         look_ahead->type == TOKEN_RIGHT_PAREN ||
         look_ahead->type == TOKEN_COMMA || look_ahead->type == TOKEN_ASSIGN_OP;
}

char *parseVar() {
  // VAR → ID VARC
  preParse("var");

  if (look_ahead->type == TOKEN_ID) {
    char *lexeme = look_ahead->lexeme;
    match(TOKEN_ID, lexeme);
    parseVarc();

    return lexeme;
  }

  handleParseError("Expected an identifier", isInFollowSetForVar);
  return NULL;
}

void parseVarc() {
  // VARC → [ EXPR ]
  // VARC → ε

  preParse("varc");

  if (look_ahead->type == TOKEN_LEFT_SQUARE_PAREN) {
    match(TOKEN_LEFT_SQUARE_PAREN, "[");
    parseExpr();
    match(TOKEN_LEFT_SQUARE_PAREN, "]");

    return;
  }

  return;
}

//< Parse Functions