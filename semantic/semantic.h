#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

// Adjust as needed
#define MAX_ENTRIES 100
#define MAX_SCOPES 2
#define MAX_ARGS 10
#define MAX_CALL_STACKS 100

#include "../common/string.h"
#include <stdbool.h>

// Handling scope and type

typedef enum { INT, DOUBLE, ERROR } DataType;

typedef enum { VARIABLE, FUNCTION } SymbolType;

typedef struct {
  int lineNumber;
  char lexeme[50];
  DataType returnType;
  SymbolType symbolType;
  int parameterCount;
  DataType parameters[10];
} SymbolTableEntry;

typedef struct {
  char name[50];
  int entryCount;
  SymbolTableEntry entries[MAX_ENTRIES];
} SymbolTable;

typedef struct {
  int argCount;
  DataType argTypes[MAX_ARGS];
} FunctionCallFrame;

typedef struct {
  FunctionCallFrame stack[MAX_CALL_STACKS];
  int top;
} FunctionCallStack;

// Exported variables
extern int scopeCount;
extern SymbolTable scopes[];

extern char semanticErrorBuffer[];
extern size_t semanticErrorBufferIndex;

extern DataType tempDeclarationReturnType; // For varlist only
extern DataType tempArgTypeList[];
extern SymbolTableEntry tempArgList[];
extern int argCount;

extern FunctionCallStack callStack;

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

// Nested Function calls
void pushCallFrame();
void popCallFrame();
FunctionCallFrame *currentCallFrame();

// Scope error handling
void scopeError(const char *message);
void semanticError(const char *message);

// Debugging
void printScope(SymbolTable *table);
void printCurrentScope();
void printEntry(SymbolTableEntry entry);
const char *dataTypeToString(DataType type);

#endif // SEMANTIC_ANALYSIS