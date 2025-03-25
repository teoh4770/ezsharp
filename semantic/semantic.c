#include <stdio.h>
#include "semantic.h"

// Add a new scope
void pushScope(const char *scopeName)
{
  if (scopeCount >= MAX_SCOPES)
  {
    // throw an error
    scopeError("Maximum scope limit reached.");
    return;
  }

  // Create an instance of symbol table
  // Add the new table to the current scope
  SymbolTable table;
  // missing scopeName assignment
  scopes[scopeCount++] = table;
}

// Remove the scope at the very top
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

SymbolTable *getSymbolTable()
{
  // error handling here
  if (scopeCount == 0)
  {
    scopeError("No scope available to retrieve");
    return NULL;
  }

  return &(scopes[scopeCount - 1]);
}

// Scope error handling
void scopeError(const char *message)
{
  fprintf(stderr, "Scope Error: %s\n", message);
  return;
}

// Insert the symbol at the current scope
// Check for redeclaration at current scope when inserting, throw error if found
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

  // Error: redeclaration of '[lexeme name]' at line x
  // Check for redeclaration at current scope
  // todo: implementing redeclaration check

  // Insert if no redeclaration
  table->entries[table->entryCount++] = entry;
}

// Search for a symbol from the most recent (top) scope to the oldest (bottom) scope.
SymbolTableEntry *lookupSymbol(const char *lexeme)
{
  // loop through the scopes
  // get the scope
  // loop through entries in the scope
  // compare lexeme with entry lexeme
  // if match, return the SymbolTableEntry

  return NULL; // Symbol not found
}