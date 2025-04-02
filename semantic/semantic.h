#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#define MAX_ENTRIES 100 // Adjust as needed
#define MAX_SCOPES 2    // Adjust as needed

#include <stdbool.h>
#include "../common/string.h"

// Handling scope and type

typedef enum
{
  INT,
  DOUBLE,
  ERROR
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
  DataType parameters[10];
} SymbolTableEntry;

// Example:
// SymbolTable table
// table.entries[table.entryCount++] = entry;
typedef struct
{
  char name[50];
  int entryCount;
  SymbolTableEntry entries[MAX_ENTRIES];
} SymbolTable;

// Exported variables
extern int scopeCount;
extern SymbolTable scopes[];

// Define the operations for symbol table stack
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
// Look for the function symbol, starting from the very top, then bottom
SymbolTableEntry *getFunctionEntry();

// Scope error handling
void scopeError(const char *message);

// Debugging
void printScope(SymbolTable *table);
void printCurrentScope();
void printEntry(SymbolTableEntry entry);
const char *dataTypeToString(DataType type);

#endif // SEMANTIC_ANALYSIS