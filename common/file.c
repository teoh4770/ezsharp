#include <fcntl.h>  // For open() flags
#include <unistd.h> // For write() and close()
#include "../common/string.h"
#include "../common/file.h"

// Todo: 1024 to BUFFER_SIZE
//> buffer-file-functions

void appendToBuffer(char *buffer, size_t *bufferIndex, const char *message, const char *fileName)
{
  size_t messageLength = _strlen(message);

  // If exceed the buffer max length
  if (*bufferIndex + messageLength >= 1024 - 1)
  {
    // Need to get filename
    flushBufferToFile(fileName, buffer, bufferIndex);
  }

  // Copy message and ensure null termination
  _strncpy(buffer + *bufferIndex, message, messageLength);
  *bufferIndex += messageLength;
  buffer[*bufferIndex] = '\0'; // Explicitly null-termination
}

int generateFile(const char *fileName, const char *content)
{
  // O_WRONLY -> Writing only
  // O_CREAT  -> Create file if it does not exist
  // O_APPEND -> Append content at the end of the file
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

  printf("Successfully writing to file: %s\n", fileName);
  close(file);

  return 0;
}

void flushBufferToFile(const char *fileName, char *buffer, size_t *bufferIndex)
{
  if (*bufferIndex > 0)
  {
    buffer[*bufferIndex] = '\0'; // Final null-termination
    generateFile(fileName, buffer);
  }
  *bufferIndex = 0; // Reset buffer
  buffer[0] = '\0'; // Clear buffer content
}

//< buffer-file-functions