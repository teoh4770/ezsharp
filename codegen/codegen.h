#ifndef CODEGEN_H
#define CODEGEN_H

#define MAX_OPERANDS 3
#define MAX_INSTRUCTIONS 1024

#include "../common/string.h"
#include "../common/token_utils.h"
#include "stdbool.h"

// Todo: complete operand type

// Define types
typedef struct {
  char name[32];
  int type;
} Operand;

typedef struct {
  int operation;
  Operand result;
  Operand arg1;
  Operand arg2;
} Instruction;

Instruction instructions[MAX_INSTRUCTIONS];
int instructionCount;

// Create a new temp variable function
// Create a new label function

void CodeGen(Token *tokens);

// Non-terminal
void prog();
void fns();
void fnsc();
void fn();
void params();
void paramsc();
void fname();
void decls();
void declsc();
void decl();
void type();
void vars();
void varsc();
void stmts();
void stmtsc();
void stmt();
void stmtc();
void expr();
void exprc();
void term();
void termc();
void factor();
void factorc();
void exprs();
void exprsc();
void var();
void varc();

// Boolean expression 
void bexpr();
void bexprc();
void bterm();
void btermc();
void bfactor();
void comp();

#endif