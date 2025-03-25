#include <stdio.h>
#include "semantic.h"

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
  if (scopeCount >= MAX_SCOPES)
  {
    char errorMsg[100];
    snprintf(errorMsg, sizeof(errorMsg), "Maximum scope limit of %d reached.", MAX_SCOPES);
    scopeError(errorMsg);
    return;
  }

  SymbolTable table;
  _strncpy(table.name, scopeName, sizeof(table.name) - 1);
  table.name[sizeof(table.name) - 1] = '\0';
  scopes[scopeCount++] = table;
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