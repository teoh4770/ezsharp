#include "semantic.h"
#include "../common/error_state.h"
#include "../common/file_utils.h"
#include <stdio.h>

#define BUFFER_SIZE 1024

// Global variables
// Scopes size of 2: global scope, function scope
int scopeCount = 0;
SymbolTable scopes[MAX_SCOPES];

char semanticErrorBuffer[BUFFER_SIZE + 1];
size_t semanticErrorBufferIndex = 0;

DataType tempDeclarationReturnType = INT;
DataType tempArgTypeList[MAX_ARGS];
SymbolTableEntry tempArgList[MAX_ARGS];
int argCount = 0;

FunctionCallStack callStack = {.top = -1}; // top property is 0 based

// Create default symbol table
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

void pushCallFrame() {
  if (callStack.top >= MAX_CALL_STACKS) {
    fprintf(stderr, "Call stack overflow\n");
    return;
  }

  callStack.top++;

  FunctionCallFrame *frame = &callStack.stack[callStack.top];
  frame->argCount = 0;

  for (int i = 0; i < MAX_ARGS; i++) {
    frame->argTypes[i] = ERROR;
  }
}

void popCallFrame() {
  if (callStack.top < 0) {
    fprintf(stderr, "Call stack is already empty\n");
    return;
  }

  callStack.top--;
}

FunctionCallFrame *currentCallFrame() {
  if (callStack.top < 0) {
    return NULL;
  }

  return &callStack.stack[callStack.top];
}

void scopeError(const char *message) {
  char fullMessage[BUFFER_SIZE + 1];
  snprintf(fullMessage, BUFFER_SIZE, "Scope Error: %s\n", message);

  semanticErrorBuffer[BUFFER_SIZE] = '\0';
  appendToBuffer(semanticErrorBuffer, &semanticErrorBufferIndex, fullMessage,
                 "semantic_errors.txt");
  flushBufferToFile("semantic_errors.txt", semanticErrorBuffer,
                    &semanticErrorBufferIndex);
}

void semanticError(const char *message) {
  setErrorOccurred();

  char fullMessage[BUFFER_SIZE + 1];
  snprintf(fullMessage, BUFFER_SIZE, "Semantic Error: %s\n", message);

  semanticErrorBuffer[BUFFER_SIZE] = '\0';
  appendToBuffer(semanticErrorBuffer, &semanticErrorBufferIndex, fullMessage,
                 "semantic_errors.txt");
  flushBufferToFile("semantic_errors.txt", semanticErrorBuffer,
                    &semanticErrorBufferIndex);
  return;
}

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