// To compile: gcc ezsharp.c lexer/*.c parser/*.c semantic/*.c -o ezsharp

//> Entry point for our compiler

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "lexer/lexer.h"
#include "parser/parser.h"

//> report-error: line, where, message
void report(int line, const char *where, const char *message)
{
  printf("[line %d] Error %s: %s", line, where, message);
}
//< report-error

//> show-error: line, message
static void error(int line, const char *message)
{
  report(line, "", message);
}
//< show-error

int main(int argc, const char *argv[])
{
  // Open the file with extension ".cp"
  int fd = open("tests/CorrectSyntax.cp", O_RDONLY);
  if (fd == -1)
  {
    printf("Error Number % d\n", errno);
    _exit(1);
  }

  // Open the transition file for lexer
  int transitionTableFd = open("lexer_transition.txt", O_RDONLY);
  if (transitionTableFd == -1)
  {
    printf("Error Number % d\n", errno);
    _exit(1);
  }

  int tokenCount = 0;
  Token *tokens = lexicalAnalysis(&fd, &transitionTableFd);
  Parse(tokens, getTokenCount());

  if (close(fd) < 0 || close(transitionTableFd) < 0)
  {
    _exit(1);
  }

  return 0;
}