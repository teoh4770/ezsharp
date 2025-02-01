#include "scanner.h"

void initScanner(Scanner *scanner, char *buffer)
{
  scanner->lexemeBegin = buffer;
  scanner->forward = buffer;
  scanner->line = 1;
  scanner->col = 0;
}

char peek(Scanner *scanner)
{
  return *scanner->forward;
}

char getNextChar(DoubleBuffer *db, Scanner *scanner)
{
  char character = peek(scanner);

  if (character == EOF)
  {
    if (&character == &db->buffer1[BUFFER_SIZE - 1])
    {
      db->activeBuffer = 1;
      fillBuffer(db);
      scanner->forward = db->buffer2;
    }
    else if (&character == &db->buffer2[BUFFER_SIZE - 1])
    {
      db->activeBuffer = 0;
      fillBuffer(db);
      scanner->forward = db->buffer1;
    }
    else
    {
      return EOF;
    }
  }
  
  return *scanner->forward++;
}