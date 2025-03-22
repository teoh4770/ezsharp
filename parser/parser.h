#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

static Token *look_ahead; // Tracks the next token in parsing

static Token identifiers[1024]; // Stores identifier tokens
static int identifierCount = 0; // Number of identifiers collected

static char syntaxErrorBuffer[BUFFER_SIZE + 1]; // Stores syntax error messages
static size_t syntaxErrorBufferIndex = 0;       // Tracks the buffer index

static char symbolTableBuffer[BUFFER_SIZE + 1]; // Stores formatted symbol table
static size_t symbolTableBufferIndex = 0;       // Tracks the buffer index

// Helper functions
void addEndToken(Token *tokens, int *tokenCount);
void preParse(const char *message);
void advanceToken();
void printSymbolTable();
void syntaxError(const char *expectedMessage);
bool matchToken(Token current, Token target);
bool isAtEnd();

// Parsing functions
bool match(Token token);
void Parse(Token *tokens, int tokenCount);

// Synchronization functions
// void syncProg();
// void syncFns();
// void syncFn();
// void syncParamsc();
// void syncFname();
// void syncDecls();
// void syncDecl();
// void syncType();
// void syncStmt();
// void syncStmtc();
// void syncFactor();
// void syncBfactor();
// void syncComp();
// void syncVar();
// void syncVarc();

// Non-terminal parsing functions
void parseProg();
void parseFns();
void parseFnsc();
void parseFn();
void parseParams();
void parseParamsc();
void parseFname();
void parseDecls();
void parseDeclsc();
void parseDecl();
void parseType();
void parseVars();
void parseVarsc();
void parseStmts();
void parseStmtsc();
void parseStmt();
void parseStmtc();
void parseExpr();
void parseExprc();
void parseTerm();
void parseTermc();
void parseFactor();
void parseFactorc();
void parseExprs();
void parseExprsc();
void parseBexpr();
void parseBexprc();
void parseBterm();
void parseBtermc();
void parseBfactor();
void parseBfactorc();
void parseComp();
void parseVar();
void parseVarc();

#endif // PARSER_H