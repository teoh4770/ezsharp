#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#define MAX_ENTRIES 100 // Adjust as needed

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

typedef struct
{
  int lineNumber;
  char lexeme[50];
  DataType returnType;
  SymbolType symbolType;
  int parameterCount;
} SymbolTableEntry;

// SymbolTable contains a list of SymbolTableEntry
typedef struct
{
  SymbolTableEntry entries[MAX_ENTRIES];
  int count; // Track the current number of entries
} SymbolTable;

#endif