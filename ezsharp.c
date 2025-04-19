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
  int fd = open("tests/CorrectSyntax.cp", O_RDONLY);
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

  // Start compiling with lexical analysis
  Token *tokens = lexicalAnalysis(&fd, &transitionTableFd);

  // Add end token at the end of the list
  tokens[tokenCount++] = makeToken(TOKEN_DOLLAR, "$", 1, -1);

  // The tokens generated by lexer is now used by parser and semantic analyser
  Parse(&tokens); 
  
  // Check if any frontend error
  if (hasError) {
    fprintf(stderr, "\nErrors encountered. Skipping code generation.\n");
    _exit(1);
  }

  // If got no frontend error, we generate code
  CodeGen(tokens);

  if (close(fd) < 0 || close(transitionTableFd) < 0) {
    _exit(1);
  }

  return 0;
}