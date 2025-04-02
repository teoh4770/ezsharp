#include "semantic.h"
#include <stdio.h>

#define BUFFER_SIZE 1024

// Global variables
// Scopes size of 2: global scope, function scope
int scopeCount = 0;
SymbolTable scopes[MAX_SCOPES];

char semanticErrorBuffer[BUFFER_SIZE + 1];
size_t semanticErrorBufferIndex = 0;

// function to create default symbol table
SymbolTable defaultSymbolTable(const char *scopeName) {
  SymbolTable table;
  table.entryCount = 0;
  // Update the name of the symbol table
  _strncpy(table.name, scopeName, sizeof(table.name) - 1);
  table.name[sizeof(table.name) - 1] = '\0';

  return table;
}

void pushScope(const char *scopeName) {
  printf("scope count before push scope: %d\n", scopeCount);

  if (scopeCount >= MAX_SCOPES) {
    char errorMsg[100];
    snprintf(errorMsg, sizeof(errorMsg), "Maximum scope limit of %d reached.",
             MAX_SCOPES);
    scopeError(errorMsg);
    return;
  }

  SymbolTable table = defaultSymbolTable(scopeName);
  scopes[scopeCount++] = table;
  printScope(&table);
}

SymbolTable *popScope() {
  if (scopeCount <= 0) {
    scopeError("No scopes left to pop");
    return NULL;
  }

  SymbolTable *popTable = &(scopes[scopeCount - 1]);
  scopeCount--;

  puts("Pop the table");
  printScope(popTable);

  return popTable;
}

SymbolTable *getSymbolTable() {
  if (scopeCount == 0) {
    scopeError("No scope available to retrieve");
    return NULL;
  }

  return &(scopes[scopeCount - 1]);
}

void insertSymbol(SymbolTableEntry entry) {
  // Get the current scope
  SymbolTable *table = getSymbolTable();
  if (table == NULL)
    return; // Handling error in getSymbolTable

  // Error handling if reach max entries
  if (table->entryCount >= MAX_ENTRIES) {
    scopeError("Maximum symbol table entries reached.");
    return;
  }

  // Check for redeclaration at current scope
  for (int i = 0; i < table->entryCount; i++) {
    if (_strcmp(table->entries[i].lexeme, entry.lexeme) == 0) {
      char errorMsg[100];
      snprintf(errorMsg, sizeof(errorMsg), "Redeclaration of '%s' at line %d",
               entry.lexeme, entry.lineNumber);
      scopeError(errorMsg);
      return;
    }
  }

  // Insert if no redeclaration
  table->entries[table->entryCount++] = entry;
}

// Perform lookup from current scope and proceed to parent scopes
SymbolTableEntry *lookupSymbol(const char *lexeme) {
  for (int i = scopeCount - 1; i >= 0; i--) {
    SymbolTable *table = &scopes[i];

    for (int j = 0; j < table->entryCount; j++) {
      printEntry(table->entries[j]);
      printf("lexeme: %s\n", lexeme);

      if (_strcmp(table->entries[j].lexeme, lexeme) == 0) {
        return &(table->entries[j]);
      }
    }
  }

  return NULL; // Symbol not found
}

SymbolTableEntry *getFunctionEntry() {
  for (int i = scopeCount - 1; i >= 0; i--) {
    SymbolTable *table = &scopes[i];

    for (int j = 0; j < table->entryCount; j++) {
      if (table->entries[j].symbolType == FUNCTION) {
        return &(table->entries[j]);
      }
    }
  }

  return NULL; // Symbol not found
}

void scopeError(const char *message) {
  fprintf(stderr, "Scope Error: %s\n", message);
  return;
}

// void semanticError(const char *expectedMessage) {
//   // char *lexeme = getTokenLexeme(look_ahead);
//   char semanticErrorMessage[BUFFER_SIZE + 1];

//   // Print and create syntax error file
//   // sprintf(semanticErrorMessage, "Semantic Error: %s, but found '%s' at
//   line
//   // %d\n",
//   //         expectedMessage, lexeme, look_ahead->line);
//   \

//   appendToBuffer(semanticErrorBuffer, &semanticErrorBufferIndex,
//                  semanticErrorMessage, "semantic_errors.txt");
//   flushBufferToFile("semantic_errors.txt", semanticErrorBuffer,
//                     &semanticErrorBufferIndex);

//   // free(lexeme);
// }

void printScope(SymbolTable *table) {
  puts("===============");
  puts("Print Scope");
  puts("===============");
  printf("table name: %s\n", table->name);
  printf("entry count: %d\n", table->entryCount);
  puts("");
  for (int i = 0; i < table->entryCount; i++) {
    printEntry(table->entries[i]);
  }
  puts("");
  puts("");
}

void printCurrentScope() {
  SymbolTable *table = &scopes[scopeCount - 1];

  puts("===============");
  puts("Print Scope");
  puts("===============");
  printf("table name: %s\n", table->name);
  printf("entry count: %d\n", table->entryCount);
  puts("");
  for (int i = 0; i < table->entryCount; i++) {
    printEntry(table->entries[i]);
  }
  puts("");
  puts("");
}

void printEntry(SymbolTableEntry entry) {
  printf("lexeme: %s\n", entry.lexeme);
  printf("line: %d\n", entry.lineNumber);
  printf("arg count: %d\n", entry.parameterCount);
  printf("return type: %s\n", entry.returnType == INT ? "integer" : "double");
  printf("symbol type: %s\n",
         entry.symbolType == VARIABLE ? "variable" : "function");

  for (int i = 0; i < entry.parameterCount; i++) {
    printf("arg %d type: %s\n", i + 1,
           entry.parameters[i] == INT ? "integer" : "double");
  }

  puts("");
}

const char *dataTypeToString(DataType type) {
  switch (type) {
  case INT:
    return "int";
  case DOUBLE:
    return "double";
  case ERROR:
    return "error";
  default:
    return "unknown";
  }
}