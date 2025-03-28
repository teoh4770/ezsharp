#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/token.h"
#include "../semantic/semantic.h"

// Exported variables
extern Token *look_ahead;
extern Token identifiers[];
extern int identifierCount;

// Helper functions
void addEndToken(Token *tokens, int *tokenCount);
void preParse(const char *message);
void advanceToken();
void syntaxError(const char *expectedMessage);
bool matchToken(Token current, Token target);
bool isAtEnd();

// Parsing functions
bool match(Token token);
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
char *parseVar();
void parseVarc();

#endif // PARSER_H