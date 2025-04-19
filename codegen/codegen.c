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
void preGen(const char *message) {
  printf("Currently generate: %s\n", message);
}

void CodeGen(Token *tokens) {
  // Reset look_ahead to the beginning of tokens again
  look_ahead = tokens;

  puts("Generating code now");

  prog();

  if (look_ahead->type == TOKEN_DOLLAR) {
    puts("=====================");
    puts("Parsing Reach To End!");
    puts("=====================");
  } else {
    puts("=========================");
    puts("Parsing Not Reach To End!");
    puts("=========================");
  }
}

void prog() {
  // PROG → A FNS DECLS STMTS B .
  // A(); // Starting point of IR
  preGen("prog");

  fns();

  decls();

  stmts();
  // B(); // Final point of IR
  matchType(TOKEN_DOT);
}

void fns() {
  // FNS → FN ; FNSC
  // FNS → ε

  preGen("fns");

  if (isKeyword("def", 3)) {
    fn();

    matchType(TOKEN_SEMICOLON);

    fnsc();

    return;
  }

  return;
}

void fnsc() {
  // FNSC → FN; FNSC
  // FNSC → ε
  preGen("fnsc");

  fns();
}

void fn() {
  // FN → def TYPE FNAME ( PARAMS ) C A C DECLS STMTS fed B
  // Todo: create label for function
  preGen("fn");

  matchKeyword("def");

  type();

  fname();

  matchType(TOKEN_LEFT_PAREN);

  params();

  matchType(TOKEN_RIGHT_PAREN);

  decls();

  stmts();

  matchKeyword("fed");
}

void params() {
  // PARAMS → TYPE VAR PARAMSC
  // PARAMS → ε
  preGen("params");

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
  preGen("paramsc");

  if (look_ahead->type == TOKEN_COMMA) {
    matchType(TOKEN_COMMA);

    type();

    var();

    paramsc();

    return;
  }

  return;
}

void fname() {
  // FNAME → ID
  preGen("fname");

  if (look_ahead->type == TOKEN_ID) {
    matchType(TOKEN_ID);
  }
}

void decls() {
  // DECLS → DECL; DECLSC
  // DECLS → ε
  preGen("decls");

  if (isKeyword("int", 3) || isKeyword("double", 6)) {
    decl();

    matchType(TOKEN_SEMICOLON);

    declsc();

    return;
  }

  return;
}

void declsc() {
  // DECLS → DECL; DECLSC
  // DECLS → ε
  preGen("declsc");

  decls();
}

void decl() {
  // DECL → TYPE VARS
  preGen("decl");

  type();
  vars();
}

void type() {
  // TYPE → int
  // TYPE → double
  // if lookAhead is int, match keyword int
  // if lookAhead is double, match keyword double
  preGen("type");

  if (isKeyword("int", 3)) {
    matchKeyword("int");
    return;
  }

  if (isKeyword("double", 3)) {
    matchKeyword("double");
    return;
  }
}

void vars() {
  // VARS → VAR C VARSC
  preGen("vars");

  var();
  varsc();
}

void varsc() {
  // VARSC → , VARS
  // VARSC → ε
  preGen("varsc");

  if (look_ahead->type == TOKEN_COMMA) {
    matchType(TOKEN_COMMA);

    vars();

    return;
  }

  return;
}

void stmts() {
  // STMTS → STMT STMTSC
  preGen("stmts");

  stmt();

  stmtsc();
}

void stmtsc() {
  // STMTSC → ; STMTS
  // STMTSC → ε
  preGen("stmtsc");

  if (look_ahead->type == TOKEN_SEMICOLON) {
    matchType(TOKEN_SEMICOLON);

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

  preGen("stmt");

  if (look_ahead->type == TOKEN_ID) {
    var();

    matchType(TOKEN_ASSIGN_OP);

    expr();

  } else if (isKeyword("if", 2)) {
    matchKeyword("if");

    bexpr();

    matchKeyword("then");

    stmts();

    stmtc();

  } else if (isKeyword("while", 5)) {
    matchKeyword("while");

    bexpr();

    matchKeyword("do");

    stmts();

    matchKeyword("od");

  } else if (isKeyword("print", 5)) {
    matchKeyword("print");

    expr();

  } else if (isKeyword("return", 6)) {
    matchKeyword("return");

    expr();

  } else {
    return;
  }
}
void stmtc() {
  // STMTC → fi
  // STMTC → else STMTS fi

   preGen("stmtc");

  if (isKeyword("fi", 2)) {
    matchKeyword("fi");
    return;
  }

  if (isKeyword("else", 4)) {
    matchKeyword("else");

    stmts();

    matchKeyword("fi");

    return;
  }
}

void expr() {
  // EXPR → TERM EXPRC
  preGen("expr");

  term();
  exprc();
}

void exprc() {
  // EXPRC → + TERM EXPRC
  // EXPRC → - TERM EXPRC
  // EXPRC → ε

  preGen("exprc");

  if (look_ahead->type == TOKEN_ADD || look_ahead->type == TOKEN_SUB) {
    matchType(look_ahead->type);

    term();

    exprc();

    return;
  }

  return;
}
void term() {
  // TERM → FACTOR TERMC
  preGen("term");

  factor();

  termc();
}

void termc() {
  // TERMC → * FACTOR TERMC
  // TERMC → / FACTOR TERMC
  // TERMC → % FACTOR TERMC
  // TERMC → ε

  preGen("termc");

  if (look_ahead->type == TOKEN_MUL || look_ahead->type == TOKEN_DIV ||
      look_ahead->type == TOKEN_MOD) {
    matchType(look_ahead->type);

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

  preGen("factor");

  if (look_ahead->type == TOKEN_ID) {
    matchType(TOKEN_ID);

    factorc();

    return;
  }

  if (isNumber(look_ahead->type)) {
    matchType(look_ahead->type);

    return;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    matchType(TOKEN_LEFT_PAREN);

    expr();

    matchType(TOKEN_RIGHT_PAREN);

    return;
  }
}

void factorc() {
  // FACTORC → VARC
  // FACTORC → ( EXPRS )

  preGen("factorc");

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    matchType(TOKEN_LEFT_PAREN);

    exprs();

    matchType(TOKEN_RIGHT_PAREN);

    return;
  }

  varc();
}

void exprs() {
  // EXPRS → EXPR EXPRSC
  // EXPRS → ε

  preGen("exprs");

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

  preGen("exprsc");

  if (look_ahead->type == TOKEN_COMMA) {
    matchType(TOKEN_COMMA);

    exprs();

    return;
  }

  return;
}

void var() {
  // VAR → ID VARC
  preGen("var");

  matchType(TOKEN_ID);

  varc();
}

void varc() {
  // VARC → [ EXPR ]
  // VARC → ε

  preGen("varc");

  if (look_ahead->type == TOKEN_LEFT_SQUARE_PAREN) {
    matchType(TOKEN_LEFT_SQUARE_PAREN);

    expr();

    matchType(TOKEN_LEFT_SQUARE_PAREN);

    return;
  }

  return;
}

void bexpr() {
  // BEXPR → BTERM BEXPRC
  preGen("bexpr");

  bterm();
  bexprc();
}

void bexprc() {
  // BEXPRC → or BTERM BEXPRC
  // BEXPRC → ε
  preGen("bexprc");

  if (isKeyword("or", 2)) {
    matchKeyword("or");

    bterm();

    bexprc();

    return;
  }

  return;
}

void bterm() {
  // BTERM → BFACTOR BTERMC
  preGen("bterm");

  bfactor();
  btermc();
}

void btermc() {
  // BTERMC → and BFACTOR BTERMC
  // BTERMC → ε
  preGen("btermc");

  if (isKeyword("and", 3)) {
    matchKeyword("and");

    bfactor();

    btermc();

    return;
  }

  return;
}

void bfactor() {
  // BFACTOR → not bfactor
  // BFACTOR → (expr comp expr)
  preGen("bfactor");

  if (isKeyword("not", 3)) {
    matchKeyword("not");

    bfactor();

    return;
  }

  if (look_ahead->type == TOKEN_LEFT_PAREN) {
    matchType(TOKEN_LEFT_PAREN);

    expr();

    comp();

    expr();

    matchType(TOKEN_RIGHT_PAREN);

    return;
  }
}

void comp() {
  preGen("comp");

  if (isComparison(look_ahead->type)) {
    matchType(look_ahead->type);
    return;
  }
}
