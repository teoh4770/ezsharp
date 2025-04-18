// parser.c: the main component to handle parsing

#include <fcntl.h>  // For open() flags
#include <stdarg.h> // For variable argument lists
#include <stdio.h>
#include <stdlib.h> // For free()
#include <unistd.h> // For write() and close()

#include "../common/file_utils.h"
#include "../common/string.h"
#include "../common/token_utils.h"
#include "parser.h"

#define BUFFER_SIZE 1024

// Global variables
Token identifiers[1024];
int identifierCount = 0;

// Variable for this module
char parseErrorBuffer[BUFFER_SIZE + 1];
size_t parseErrorBufferIndex = 0;

char symbolTableBuffer[BUFFER_SIZE + 1];
size_t symbolTableBufferIndex = 0;

//> Helper Functions
void preParse(const char *message) {
  printf("Currently parsing: %s\n", message);
}

void parseError(const char *expectedMessage) {
  char *lexeme = getTokenLexeme(look_ahead);
  char parseErrorMessage[BUFFER_SIZE + 1];

  // Print and create syntax error file
  sprintf(parseErrorMessage, "Syntax Error: %s, but found '%s' at line %d\n",
          expectedMessage, lexeme, look_ahead->line);

  appendToBuffer(parseErrorBuffer, &parseErrorBufferIndex, parseErrorMessage,
                 "syntax_analysis_errors.txt");
  flushBufferToFile("syntax_analysis_errors.txt", parseErrorBuffer,
                    &parseErrorBufferIndex);

  free(lexeme);
}

void handleParseError(const char *message, bool (*isInFollowSet)()) {
  parseError(message);

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
SymbolTableEntry *D(const char *lexeme) {
  SymbolTableEntry *variable = lookupSymbol(lexeme);

  if (!variable) {
    handleSemanticError("Undeclared variable %s at line number %d", lexeme,
                        look_ahead->line);
  }

  return variable;
}

void handleSemanticError(const char *format, ...) {
  char semanticErrorMessage[BUFFER_SIZE + 1];
  va_list args;

  va_start(args, format);
  vsnprintf(semanticErrorMessage, BUFFER_SIZE, format, args);
  va_end(args);

  semanticError(semanticErrorMessage);
}

void resetArgCount() { argCount = 0; }

void handleFunctionCall(SymbolTableEntry *symbol) {
  SymbolTableEntry *functionEntry = lookupSymbol(symbol->lexeme);
  FunctionCallFrame *frame = currentCallFrame();

  if (frame->argCount != functionEntry->parameterCount) {
    if (frame->argCount > functionEntry->parameterCount) {
      handleSemanticError("%s arguments for function '%s' (line %d). Expected "
                          "%d, but got %d.",
                          "Too many", functionEntry->lexeme, look_ahead->line,
                          functionEntry->parameterCount, frame->argCount);
    } else {
      handleSemanticError("%s arguments for function '%s' (line %d). Expected "
                          "%d, but got %d.",
                          "Too few", functionEntry->lexeme, look_ahead->line,
                          functionEntry->parameterCount, frame->argCount);
    }
  }

  for (int i = 0; i < functionEntry->parameterCount; i++) {
    if (frame->argTypes[i] != functionEntry->parameters[i]) {
      handleSemanticError("Argument %d of function '%s' (line %d) has "
                          "incorrect type. Expected '%s', but got '%s'.",
                          i + 1, functionEntry->lexeme, look_ahead->line,
                          dataTypeToString(functionEntry->parameters[i]),
                          dataTypeToString(frame->argTypes[i]));
    }
  }

  resetArgCount();
  popCallFrame();
}

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
  remove("semantic_errors.txt");

  // Initialization
  parseErrorBuffer[BUFFER_SIZE] = '\0';
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
    parseError("Expected '.' to indicate end of the program");
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

  // Insert function symbol at global scope
  C(FUNCTION, type, look_ahead->line, argCount, funcName);

  // Insert function scope
  A(funcName);

  // Insert argument symbol at function scope
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

    if (!match(TOKEN_ASSIGN_OP, "=")) {
      handleParseError("Expected '=' for assignment", isInFollowSetForStmt);
      return;
    }

    // Get the return type of expression
    DataType rightType = parseExpr();

    // Type checking: Ensure LHS (variable) type matches RHS (expression) type
    if (variable && variable->returnType != rightType) {
      handleSemanticError("Type mismatch during assignment at line %d. "
                          "Left operand is '%s', but right operand is '%s'.",
                          look_ahead->line,
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
      handleSemanticError("Function declared as %s but returning %s",
                          dataTypeToString(functionEntry->returnType),
                          dataTypeToString(returnType));
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
      handleSemanticError("Type mismatch in arithmetic operation at line %d. "
                          "Left operand is '%s', but right operand is '%s'.",
                          opToken->line, dataTypeToString(leftType),
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
    Token *opToken = look_ahead;
    match(look_ahead->type, look_ahead->lexeme);

    DataType rightType = parseFactor();

    if (leftType != rightType) {
      handleSemanticError("Type mismatch in arithmetic operation at line %d. "
                          "Left operand is '%s', but right operand is '%s'.",
                          opToken->line, dataTypeToString(leftType),
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
    char *factorId = look_ahead->lexeme;

    match(TOKEN_ID, factorId);
    SymbolTableEntry *symbol = D(factorId);
    parseFactorc(symbol);

    return symbol->returnType;
  }

  if (isNumber(look_ahead->type)) {
    DataType numberType = look_ahead->type == TOKEN_INT ? INT : DOUBLE;

    match(look_ahead->type, look_ahead->lexeme);

    return numberType;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    match(TOKEN_LEFT_PAREN, "(");
    DataType exprType = parseExpr();

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
      handleSemanticError("'%s' is not a function but is used as one (line "
                          "%d).",
                          symbol->lexeme, look_ahead->line);
    }

    // Manage call stack
    pushCallFrame();

    match(TOKEN_LEFT_PAREN, "(");
    parseExprs();

    if (!match(TOKEN_RIGHT_PAREN, ")")) {
      handleParseError("Expected closing parenthesis ')'",
                       isInFollowSetForFactor);
      return;
    }
    handleFunctionCall(symbol);

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
    FunctionCallFrame *frame = currentCallFrame();

    frame->argTypes[frame->argCount++] = parseExpr();
    parseExprsc();

    return;
  }

  return;
}

void parseExprsc() {
  // EXPRSC → , EXPRS
  // EXPRSC → ε
  preParse("exprsc");

  if (look_ahead->type == TOKEN_COMMA) {
    match(TOKEN_COMMA, ",");
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
  // BFACTOR → (expr comp expr)
  preParse("bfactor");

  if (isKeyword("not", 3)) {
    match(TOKEN_KEYWORD, "not");
    parseBfactor();
    return;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    match(TOKEN_LEFT_PAREN, "(");

    DataType leftType = parseExpr();
    parseComp();
    DataType rightType = parseExpr();

    if (leftType != rightType) {
      handleSemanticError("Type mismatched in comparison at line %d. Left "
                          "operand is '%s' but right operand is '%s'",
                          look_ahead->line, dataTypeToString(leftType),
                          dataTypeToString(rightType));
    }

    if (!match(TOKEN_RIGHT_PAREN, ")")) {
      handleParseError("Expected closing parenthesis ')'",
                       isInFollowSetForBfactor);
      return;
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