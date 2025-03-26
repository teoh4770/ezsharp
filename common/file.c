#include <fcntl.h>  // For open() flags
#include <unistd.h> // For write() and close()
#include "../common/string.h"

// Todo: 1024 to BUFFER_SIZE
//> buffer-file-functions

void appendToBuffer(char *buffer, size_t *bufferIndex, const char *message)
{
  size_t messageLength = _strlen(message);

  // Handle buffer overflow? (later)
  if (*bufferIndex + messageLength >= 1024 - 1)
  {
    // flush the buffer earlier or log an error here
    return;
  }

  // Add message to the end of the buffer
  _strncpy(buffer + *bufferIndex, message, messageLength);
  *bufferIndex += messageLength;
}

int generateFile(const char *fileName, const char *content)
{
  // O_WRONLY -> Writing only
  // O_CREAT  -> Create file if it does not exist
  // O_APPEND -> Append data at the end of the file
  // S_IRUSR  -> Set the read permission for user
  // S_IWUSR  -> Set the write permission for user
  int file = open(fileName, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (file == -1)
  {
    perror("Failed to open file for writing");
    return -1;
  }

  printf("%s\n", content);

  size_t contentLength = _strlen(content);
  if (contentLength == 0)
  {
    close(file);
    return 0;
  }

  size_t bytesWrite = write(file, content, contentLength);
  if (bytesWrite == -1)
  {
    perror("Failed to write to file");
    close(file);
    return -1;
  }

  // printf("Successfully writing file: %s\n", fileName);
  close(file);

  return 0;
}

void flushBufferToFile(const char *fileName, char *buffer, size_t *bufferIndex)
{
  generateFile(fileName, buffer);
  *bufferIndex = 0; // Reset buffer
}

//< buffer-file-functions