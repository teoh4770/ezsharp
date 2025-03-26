#ifndef FILE_H
#define FILE_H

#include <stdio.h>

void appendToBuffer(char *buffer, size_t *bufferIndex, const char *message);
int generateFile(const char *fileName, const char *content);
void flushBufferToFile(const char *fileName, char *buffer, size_t *bufferIndex);

#endif