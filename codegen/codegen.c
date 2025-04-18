#include "codegen.h"
#include "../common/token_utils.h"

Instruction instructions[MAX_INSTRUCTIONS];
int instructionCount;

// Create a new temp variable function
// Create a new label function

// Aim: generate 3TAC version of source code that is already correct

// Note: symbol table has been created, now might need to refer for quick
// reference when creating temporary variable.

// Note: Go through the tokens again
// Should generate 3TAC for any code that is error free
// Output Intermediate Code File

// Token methods
//> Parse Functions
void CodeGen(Token *tokens) {
  look_ahead = tokens;
  
  // prog();
}

void prog() {
  // PROG → A FNS DECLS STMTS B .
  // A(); // Starting point of IR
  fns();
  decls();
  stmts();
  // B(); // Final point of IR
}

void fns() {
  // FNS → FN ; FNSC
  // FNS → ε

  // match def
  if (isKeyword("def", 3)) {
    fn();
    match(TOKEN_SEMICOLON, ";");
    fnsc();

    return;
  }

  return;
}

void fnsc() {
  // FNSC → FN; FNSC
  // FNSC → ε

  fns();
}

void fn() {
  // FN → def TYPE FNAME ( PARAMS ) C A C DECLS STMTS fed B
  // Todo: create label for function
  match(TOKEN_KEYWORD, "def");
  type();
  fname();
  match(TOKEN_LEFT_PAREN, "(");
  params();
  match(TOKEN_LEFT_PAREN, ")");
  decls();
  stmts();
  match(TOKEN_KEYWORD, "fed");
}

void params() {
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    type();
    var();
    paramsc();

    return;
  }

  return;
}

void paramsc() {
  // PARAMSC → , TYPE VAR PARAMSC
  // PARAMSC → ε
  if (match(TOKEN_COMMA, ",")) {
    type();
    var();
    paramsc();

    return;
  }

  return;
}

void fname() {
  // FNAME → ID
  // match(TOKEN_ID, lexeme);
}

void decls() {
  // DECLS → DECL; DECLSC
  // DECLS → ε
  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    decl();
    match(TOKEN_SEMICOLON, ";");
    declsc();

    return;
  }

  return;
}

void declsc() {
  // DECLS → DECL; DECLSC
  // DECLS → ε
  decls();
}

void decl() {
  // DECL → TYPE VARS
  type();
  vars();
}

void type() {
  // TYPE → int
  // TYPE → double
  // if lookAhead is int, match keyword int
  // if lookAhead is double, match keyword double
}

void vars() {
  // VARS → VAR C VARSC
  var();
  varsc();
}

void varsc() {
  // VARSC → , VARS
  // VARSC → ε
  if (match(TOKEN_COMMA, ",")) {
    vars();
    return;
  }

  return;
}

void stmts() {
  // STMTS → STMT STMTSC
  stmt();
  stmtsc();
}

void stmtsc() {
  // STMTSC → ; STMTS
  // STMTSC → ε
  if (match(TOKEN_SEMICOLON, ";")) {
    stmts();
    return;
  }

  return;
}

void stmt() {
  // STMT → VAR D = EXPR
  // STMT → if BEXPR then STMTS STMTC
  // STMT → while BEXPR do STMTS od
  // STMT → print EXPR
  // STMT -> return EXPR
  // STMT →  ε

  // look ahead type is id
  var();
  match(TOKEN_ASSIGN_OP, "=");
  expr();

  // if (isKeyword("if", 2)
  bexpr();
  match(TOKEN_KEYWORD, "then");
  stmts();
  stmtc();

  // match(TOKEN_KEYWORD, "while")
  // Todo: create labels for loop
  bexpr();
  match(TOKEN_KEYWORD, "do");
  stmts();
  match(TOKEN_KEYWORD, "od");

  // match(TOKEN_KEYWORD, "print")
  expr();

  // match(TOKEN_KEYWORD, "return")
  expr();

  return;
}
void stmtc() {
  // STMTC → fi
  // STMTC → else STMTS fi

  // if look ahead is fi
  match(TOKEN_KEYWORD, "fi");
  return;

  // if look ahead is else
  match(TOKEN_KEYWORD, "else");
  stmts();
  match(TOKEN_KEYWORD, "fi");
}

void expr() {
  // EXPR → TERM EXPRC
  term();
  exprc();
}

void exprc() {
  // EXPRC → + TERM EXPRC
  // EXPRC → - TERM EXPRC
  // EXPRC → ε

  if (look_ahead->type == TOKEN_ADD || look_ahead->type == TOKEN_SUB) {
    match(look_ahead->type, look_ahead->lexeme);
    term();
    exprc();

    return;
  }

  return;
}
void term() {
  // TERM → FACTOR TERMC
  factor();
  termc();
}

void termc() {
  // TERMC → * FACTOR TERMC
  // TERMC → / FACTOR TERMC
  // TERMC → % FACTOR TERMC
  // TERMC → ε

  if (look_ahead->type == TOKEN_MUL || look_ahead->type == TOKEN_DIV ||
      look_ahead->type == TOKEN_MOD) {
    match(look_ahead->type, look_ahead->lexeme);
    factor();
    termc();

    return;
  }

  return;
}

void factor() {
  // FACTOR → ID D FACTORC
  // FACTOR → NUMBER
  // FACTOR → (EXPR)

  if (look_ahead->type == TOKEN_ID) {
    // match(TOKEN_ID, factorId);
    factorc();

    return;
  }

  if (isNumber(look_ahead->type)) {
    match(look_ahead->type, look_ahead->lexeme);

    return;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    match(TOKEN_LEFT_PAREN, "(");
    expr();
    match(TOKEN_RIGHT_PAREN, ")");

    return;
  }
}

void factorc() {
  // FACTORC → VARC
  // FACTORC → ( EXPRS )

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    match(TOKEN_LEFT_PAREN, "(");
    exprs();
    match(TOKEN_RIGHT_PAREN, ")");

    return;
  }

  varc();
}

void exprs() {
  // EXPRS → EXPR EXPRSC
  // EXPRS → ε

  if (look_ahead->type == TOKEN_ID || look_ahead->type == TOKEN_INT ||
      look_ahead->type == TOKEN_DOUBLE ||
      look_ahead->type == TOKEN_LEFT_PAREN) {
    expr();
    exprsc();

    return;
  }

  return;
}

void exprsc() {
  // EXPRSC → , EXPRS
  // EXPRSC → ε

  if (look_ahead->type == TOKEN_COMMA) {
    match(TOKEN_COMMA, ",");
    exprs();

    return;
  }

  return;
}

void var() {
  // VAR → ID VARC

  // match(TOKEN_ID, lexeme);
  varc();
}

void varc() {
  // VARC → [ EXPR ]
  // VARC → ε

  if (look_ahead->type == TOKEN_LEFT_SQUARE_PAREN) {
    match(TOKEN_LEFT_SQUARE_PAREN, "[");
    expr();
    match(TOKEN_LEFT_SQUARE_PAREN, "]");

    return;
  }

  return;
}

void bexpr() {
  // BEXPR → BTERM BEXPRC
  bterm();
  bexprc();
}

void bexprc() {
  // BEXPRC → or BTERM BEXPRC
  // BEXPRC → ε
  if (match(TOKEN_KEYWORD, "or")) {
    bterm();
    bexprc();

    return;
  }

  return;
}

void bterm() {
  // BTERM → BFACTOR BTERMC
  bfactor();
  btermc();
}

void btermc() {
  // BTERMC → and BFACTOR BTERMC
  // BTERMC → ε
  if (match(TOKEN_KEYWORD, "and")) {
    bfactor();
    btermc();

    return;
  }

  return;
}

void bfactor() {
  // BFACTOR → not bfactor
  // BFACTOR → (expr comp expr)
  if (isKeyword("not", 3)) {
    match(TOKEN_KEYWORD, "not");
    bfactor();

    return;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    match(TOKEN_LEFT_PAREN, "(");
    expr();
    comp();
    expr();
    match(TOKEN_RIGHT_PAREN, ")");

    return;
  }
}

void comp() {
  if (isComparison(look_ahead->type)) {
    match(look_ahead->type, look_ahead->lexeme);
    return;
  }
}
