#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_SIZE 1024

#include <sys/types.h>

typedef struct {
  char buffer1[BUFFER_SIZE];
  char buffer2[BUFFER_SIZE];
  int fd;
  int activeBuffer;
  int fileEnd;
} DoubleBuffer;

void initDoubleBuffer(DoubleBuffer *db, int *fd);
ssize_t fillBuffer(DoubleBuffer *db);

#endif