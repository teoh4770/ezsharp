#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

// we have tokens from the Lexer object
// we can return the tokens as output from lexical analysis as well

// Global variable to track the next token
static Token *look_ahead;

// Output symbol table (actually a list of identifiers)
static Token identifiers[1024];
static int identifierCount = 0;

// Error
static char syntaxErrorBuffer[BUFFER_SIZE + 1];
static size_t syntaxErrorBufferIndex = 0;

// Symbol Table
static char symbolTableBuffer[BUFFER_SIZE + 1];
static size_t symbolTableBufferIndex = 0;

/**
 * Check if current look_ahead token match the current token
 * */
bool match(Token token);

/**
 * Entry point to start parsing, it keeps taking the tokens provided by Lexical analyser in order to implement parsing.
 */
void Parse(Token *tokens, int tokenCount);

/**
 * Parses the program, which includes function declarations, variable declarations, and statements.
 * Example: "def int main() { int x; x = 5; print x; }"
 */
void parseProg();

/**
 * Parses function declarations.
 * Example: "def int foo() { return 1; }; def int bar() { return 2; };"
 */
void parseFns();

/**
 * Parses function declarations continuation.
 * Example: "def int foo() { return 1; }; def int bar() { return 2; };"
 */
void parseFnsc();

/**
 * Parses a function definition, including its parameters, body, and return statement.
 * Example: "def int main() { int x; x = 5; return x; }"
 */
void parseFn();

/**
 * Parses function parameters inside parentheses.
 * Example: "(int x, double y)"
 */
void parseParams();

/**
 * Parses additional parameters separated by commas.
 * Example: ", double y"
 */
void parseParamsc();

/**
 * Parses a function name.
 * Example: "main"
 */
void parseFname();

/**
 * Parses variable declarations, which can be multiple and separated by semicolons.
 * Example: "int x; double y;"
 */
void parseDecls();

/**
 * Parses additional variable declarations.
 * Example: "int x; double y;"
 */
void parseDeclsc();

/**
 * Parses a single variable declaration, consisting of a type followed by variable names.
 * Example: "int x, y, z"
 */
void parseDecl();

/**
 * Parses a type declaration, such as int or double.
 * Example: "int"
 */
void parseType();

/**
 * Parses a list of variables in a declaration.
 * Example: "x, y, z"
 */
void parseVars();

/**
 * Parses additional variables separated by commas.
 * Example: ", y, z"
 */
void parseVarsc();

/**
 * Parses a sequence of statements, each separated by semicolons.
 * Example: "x = 5; print x;"
 */
void parseStmts();

/**
 * Parses additional statements following a semicolon.
 * Example: "; print x;"
 */
void parseStmtsc();

/**
 * Parses a single statement, such as assignments, conditionals, loops, and function calls.
 * Example: "if x > 5 then x = 10; fi"
 */
void parseStmt();

/**
 * Parses optional statements after an if-statement.
 * Example: "else print x; fi"
 */
void parseStmtc();

/**
 * Parses an arithmetic expression.
 * Example: "5 + x * 2"
 */
void parseExpr();

/**
 * Parses additional arithmetic operations in an expression.
 * Example: "+ x * 2"
 */
void parseExprc();

/**
 * Parses a term in an arithmetic expression, typically a factor or a multiplication/division operation.
 * Example: "x * 2"
 */
void parseTerm();

/**
 * Parses additional multiplication, division, or modulo operations in a term.
 * Example: "* 2"
 */
void parseTermc();

/**
 * Parses a factor, which can be a variable, number, or parenthesized expression.
 * Example: "(x + 5)"
 */
void parseFactor();

/**
 * Parses additional components of a factor, such as function calls or array indexing.
 * Example: "(5, y)"
 */
void parseFactorc();

/**
 * Parses a list of expressions in function arguments.
 * Example: "x + 2, y * 3"
 */
void parseExprs();

/**
 * Parses additional expressions separated by commas.
 * Example: ", y * 3"
 */
void parseExprsc();

/**
 * Parses a boolean expression used in conditionals and loops.
 * Example: "x > 5 or y < 3"
 */
void parseBexpr();

/**
 * Parses additional boolean terms connected with "or".
 * Example: "or y < 3"
 */
void parseBexprc();

/**
 * Parses a boolean term, which consists of boolean factors combined with "and".
 * Example: "x > 5 and y < 3"
 */
void parseBterm();

/**
 * Parses additional boolean factors connected with "and".
 * Example: "and y < 3"
 */
void parseBtermc();

/**
 * Parses a boolean factor, which can be a negated condition or a comparison.
 * Example: "(x < y)"
 */
void parseBfactor();

/**
 * Parses additional boolean factor components in complex boolean expressions.
 * Example: "and z > 2"
 */
void parseBfactorc();

/**
 * Parses a comparison operator.
 * Example: ">="
 */
void parseComp();

/**
 * Parses a variable, which may include array indexing.
 * Example: "arr[5]"
 */
void parseVar();

/**
 * Parses additional components of a variable, such as array indices.
 * Example: "[5]"
 */
void parseVarc();

#endif // PARSER_H
