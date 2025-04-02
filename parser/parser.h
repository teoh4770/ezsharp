#ifndef PARSER_H
#define PARSER_H

#include "../lexer/token.h"
#include "../semantic/semantic.h"
#include <stdbool.h>

// Exported variables
extern Token *look_ahead;
extern Token identifiers[];
extern int identifierCount;

// Helper functions
void addEndToken(Token *tokens, int *tokenCount);
void preParse(const char *message);
void advanceToken();
Token *previousToken();
Token *nextToken();
void syntaxError(const char *expectedMessage);
bool matchToken(Token current, Token target);
bool isAtEnd();
bool isKeyword(const char *keyword, int length);
bool isComparison(TokenType type);
bool isNumber(TokenType type);
void handleParseError(const char *message, bool (*isInFollowSet)());

// Parsing functions
bool match(TokenType tokenType, char *lexeme);
void Parse(Token *tokens, int tokenCount);

// Follow Set functions
void syncProg();
bool isInFollowSetForFns();
bool isInFollowSetForFn();
bool isInFollowSetForParamsc();
bool isInFollowSetForFname();
bool isInFollowSetForDecls();
bool isInFollowSetForDecl();
bool isInFollowSetForType();
bool isInFollowSetForStmt();
bool isInFollowSetForFactor();
bool isInFollowSetForBfactor();
bool isInFollowSetForComp();
bool isInFollowSetForVar();

// Non-terminal parsing functions
void parseProg();
void parseFns();
void parseFnsc();
void parseFn();
void parseParams();
void parseParamsc();
char *parseFname();
void parseDecls();
void parseDeclsc();
void parseDecl();
int parseType();
void parseVars();
void parseVarsc();
void parseStmts();
void parseStmtsc();
void parseStmt();
void parseStmtc();
DataType parseExpr();
DataType parseExprc(DataType leftType);
DataType parseTerm();
DataType parseTermc(DataType leftType);
DataType parseFactor();
void parseFactorc(SymbolTableEntry *symbol);
void parseExprs();
void parseExprsc();
char *parseVar();
void parseVarc();

// Boolean expression parsing functions
void parseBexpr();
void parseBexprc();
void parseBterm();
void parseBtermc();
void parseBfactor(); // Final one
void parseComp();
bool isComparison(TokenType type);

// Semantic Analysis Handlers (Markers)
// A: Insert Scope
// B: Pop Scope
// C: Insert Symbol
// D: Lookup Symbol
void A(char *scopeName);
void B();
void C(SymbolType symbolType, DataType returnType, int lineNumber,
       int parameterCount, char *symbolName);
SymbolTableEntry *D(const char *lexeme);

#endif // PARSER_H