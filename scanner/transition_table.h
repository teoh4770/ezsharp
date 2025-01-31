#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#define TT_ROWS 29
#define TT_COLS 128

//> transition-state: 29 states, from index 0 to 28, following the transition table
typedef enum
{
  STATE_START,
  STATE_LT,
  STATE_LE,
  STATE_NE,
  STATE_ASSIGN_OP,
  STATE_EQ,
  STATE_GT,
  STATE_GE,
  STATE_KEYWORD,
  STATE_ID,
  STATE_SEMICOLON,
  STATE_COMMA,
  STATE_LEFT_PAREN,
  STATE_RIGHT_PAREN,
  STATE_DOT,
  STATE_WHITESPACE,
  STATE_ADD,
  STATE_MUL,
  STATE_DIV,
  STATE_MOD,
  STATE_SUB,
  STATE_INT,
  STATE_INT_DOT,
  STATE_DOUBLE,
  STATE_NUM_E_ONLY,
  STATE_NUM_E_COEFFICIENT_ONLY,
  STATE_NUM_E,
  STATE_LEFT_SQUARE_PAREN,
  STATE_RIGHT_SQUARE_PAREN,
} TransitionState;
//< transition-state

void initTransitionTable(int rows, int cols, int arr[][TT_COLS], int *fd);

#endif