// To compile: gcc ezsharp.c lexer/*.c parser/*.c semantic/*.c codegen/*.c
// common/*.c -o ezsharp

//> Entry point for our compiler

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "codegen/codegen.h"
#include "common/error_state.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, const char *argv[]) {
  // Open the file with extension ".cp"
  // CorrectSyntaxTest
  // IncorrectSyntaxTest
  int fd = open("tests/ReturnTypeMismatched.cp", O_RDONLY);
  if (fd == -1) {
    printf("Error Number % d\n", errno);
    _exit(1);
  }

  // Open the transition file for lexer
  int transitionTableFd = open("lexer_transition.txt", O_RDONLY);
  if (open("lexer_transition.txt", O_RDONLY) == -1) {
    printf("Error Number % d\n", errno);
    _exit(1);
  }

  // Start compiling
  Token *tokens = lexicalAnalysis(&fd, &transitionTableFd);
  addEndToken(tokens, &tokenCount);
  Parse(tokens); // Output of lexer is an input for parser

  // Skip code gen if contains error during each step of compiling
  if (hasError) {
    fprintf(stderr, "\nErrors encountered. Skipping code generation.\n");
    _exit(1);
  }

  // Skip gen
  CodeGen(tokens);

  if (close(fd) < 0 || close(transitionTableFd) < 0) {
    _exit(1);
  }

  return 0;
}