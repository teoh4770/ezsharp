#include <stdio.h>
#include <unistd.h>

#include "buffer.h"

//> init-double-buffer
void initDoubleBuffer(DoubleBuffer *db, int *fd) {
  db->buffer1[BUFFER_SIZE - 1] = EOF;
  db->buffer2[BUFFER_SIZE - 1] = EOF;
  db->fd = *fd;
  db->activeBuffer = 0;
  db->fileEnd = 0;
}
//< init-double-buffer

//> fill-buffer
ssize_t fillBuffer(DoubleBuffer *db) {
  if (db->fileEnd) {
    return 0;
  }

  char *activeBuffer = db->activeBuffer == 0 ? db->buffer1 : db->buffer2;
  ssize_t bytesRead = read(db->fd, activeBuffer, BUFFER_SIZE - 1);

  if (bytesRead < BUFFER_SIZE - 1) {
    db->fileEnd = 1;
    activeBuffer[bytesRead] = EOF;
  }

  return bytesRead;
}
//< fill-buffer