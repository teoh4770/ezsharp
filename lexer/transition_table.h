#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#define TT_ROWS 29
#define TT_COLS 128

//> transition-state: 29 states, from index 0 to 28, following the transition table
typedef enum
{
  STATE_ERROR = -1,
  STATE_START = 0,
  STATE_LT = 1,
  STATE_LE = 2,
  STATE_NE = 3,
  STATE_ASSIGN_OP = 4,
  STATE_EQ = 5,
  STATE_GT = 6,
  STATE_GE = 7,
  STATE_KEYWORD = 8,
  STATE_ID = 9,
  STATE_SEMICOLON = 10,
  STATE_COMMA = 11,
  STATE_LEFT_PAREN = 12,
  STATE_RIGHT_PAREN = 13,
  STATE_DOT = 14,
  STATE_WHITESPACE = 15,
  STATE_ADD = 16,
  STATE_MUL = 17,
  STATE_DIV = 18,
  STATE_MOD = 19,
  STATE_SUB = 20,
  STATE_INT = 21,
  STATE_INT_DOT = 22,
  STATE_DOUBLE = 23,
  STATE_NUM_E_ONLY = 24,
  STATE_NUM_E_COEFFICIENT_ONLY = 25,
  STATE_NUM_E = 26,
  STATE_LEFT_SQUARE_PAREN = 27,
  STATE_RIGHT_SQUARE_PAREN = 28,
} TransitionState;

//< transition-state

void initTransitionTable(TransitionState arr[TT_ROWS][TT_COLS], int *fd);

#endif