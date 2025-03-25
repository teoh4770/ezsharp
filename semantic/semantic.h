#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#define MAX_ENTRIES 100 // Adjust as needed
#define MAX_SCOPES 2    // Adjust as needed

#include "../common/string.h"

// Handling scope and type

typedef enum
{
  INT,
  DOUBLE
} DataType;

typedef enum
{
  VARIABLE,
  FUNCTION
} SymbolType;

// Example:
// SymbolTableEntry entry = { 10, "a", INT, VARIABLE, 0 };
// SymbolTableEntry entry = { 12, "sum", INT, FUNCTION, 2 };
typedef struct
{
  int lineNumber;
  char lexeme[50];
  DataType returnType;
  SymbolType symbolType;
  int parameterCount;
} SymbolTableEntry;

// Example:
// SymbolTable table
// table.entries[table.entryCount++] = entry;
typedef struct
{
  SymbolTableEntry entries[MAX_ENTRIES];
  int entryCount; // Track the current number of entries
  char name[50];  // Scope name
} SymbolTable;

// Define the operations for symbol table stack
// - push scope: enter a new scope
// - pop scope: discard a scope
// - insert symbol: add entry to current scope
// - lookup symbol: to access information related to given name

// Scopes size of 2: global scope (create at the beginning, destroy at the end), function scope (create at the beginning of def, destroy at fed)
// One scope = one symbol table
SymbolTable scopes[MAX_SCOPES];
int scopeCount = 0;

// Scope error handling
void scopeError(const char *message);
// Add a new scope
void pushScope(const char *scopeName);
// Remove the scope at the very top
SymbolTable *popScope();
// Get the scope at the very top
SymbolTable *getSymbolTable();
// Insert the symbol at the current scope
void insertSymbol(SymbolTableEntry entry);
// Look for the symbol, starting from the very top, then bottom
SymbolTableEntry *lookupSymbol(const char *lexeme);

#endif