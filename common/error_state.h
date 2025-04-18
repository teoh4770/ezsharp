// Tracking whether we encounter either lexer, syntax or semantic error

#ifndef ERROR_STATE_H
#define ERROR_STATE_H

#include "stdbool.h"

// Global error flag
extern bool hasError;

void setErrorOccurred();

#endif