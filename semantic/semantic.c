#include <stdio.h>
#include "semantic.h"

// Global variables
// Scopes size of 2: global scope (create at the beginning, destroy at the end), function scope (create at the beginning of def, destroy at fed)
// One scope = one symbol table
int scopeCount = 0;
SymbolTable scopes[MAX_SCOPES];

// function to create default symbol table
SymbolTable defaultSymbolTable(const char *scopeName)
{
  SymbolTable table;
  table.entryCount = 0;
  // Update the name of the symbol table
  _strncpy(table.name, scopeName, sizeof(table.name) - 1);
  table.name[sizeof(table.name) - 1] = '\0';

  return table;
}

void printScope(SymbolTable *table)
{
  puts("===============");
  puts("Print Scope");
  puts("===============");
  printf("table name: %s\n", table->name);
  printf("entry count: %d\n", table->entryCount);
  for (int i = 0; i < table->entryCount; i++)
  {
    printEntry(table->entries[i]);
  }
  puts("");
  puts("");
}

void printEntry(SymbolTableEntry entry)
{
  printf("lexeme: %s\n", entry.lexeme);
  printf("line: %d\n", entry.lineNumber);
  printf("arg count: %d\n", entry.parameterCount);
  printf("return type: %d\n", entry.returnType);
  printf("symbol type: %d\n", entry.symbolType);
  puts("");
}

void scopeError(const char *message)
{
  fprintf(stderr, "Scope Error: %s\n", message);
  return;
}

SymbolTable *getSymbolTable()
{
  if (scopeCount == 0)
  {
    scopeError("No scope available to retrieve");
    return NULL;
  }

  return &(scopes[scopeCount - 1]);
}

void pushScope(const char *scopeName)
{
  // if (scopeCount >= MAX_SCOPES)
  // {
  //   char errorMsg[100];
  //   snprintf(errorMsg, sizeof(errorMsg), "Maximum scope limit of %d reached.", MAX_SCOPES);
  //   scopeError(errorMsg);
  //   return;
  // }

  SymbolTable table = defaultSymbolTable(scopeName);
  // scopes[scopeCount++] = table;
  // printScope(&table);
}

SymbolTable *popScope()
{
  if (scopeCount <= 0)
  {
    scopeError("No scopes left to pop");
    return NULL;
  }

  SymbolTable *popTable = &(scopes[scopeCount - 1]);
  scopeCount--;

  printScope(popTable);

  return popTable;
}

void insertSymbol(SymbolTableEntry entry)
{
  // Get the current scope
  SymbolTable *table = getSymbolTable();
  if (table == NULL)
    return; // Handling error in getSymbolTable

  // Error handling if reach max entries
  if (table->entryCount >= MAX_ENTRIES)
  {
    scopeError("Maximum symbol table entries reached.");
    return;
  }

  // Check for redeclaration at current scope
  for (int i = 0; i < table->entryCount; i++)
  {
    if (_strcmp(table->entries[i].lexeme, entry.lexeme) == 0)
    {
      char errorMsg[100];
      snprintf(errorMsg, sizeof(errorMsg), "Redeclaration of '%s' at line %d", entry.lexeme, entry.lineNumber);
      scopeError(errorMsg);
      return;
    }
  }

  // Insert if no redeclaration
  table->entries[table->entryCount++] = entry;
  printEntry(entry);
  printf("entry count: %d", table->entryCount);
}

SymbolTableEntry *lookupSymbol(const char *lexeme)
{
  for (int i = scopeCount - 1; i >= 0; i--)
  {
    SymbolTable *table = &scopes[i];

    for (int j = 0; j < table->entryCount; j++)
    {
      if (_strcmp(table->entries[j].lexeme, lexeme) == 0)
      {
        return &(table->entries[j]);
      }
    }
  }

  return NULL; // Symbol not found
}