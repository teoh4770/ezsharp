#include <stdio.h>
#include <unistd.h>
#include "transition_table.h"

#define BUFFER_SIZE 1024

void initTransitionTable(int rows, int cols, int arr[][TT_COLS], int *fd)
{
  char buffer[BUFFER_SIZE];
  int row = 0, col = 0;
  int num = 0;
  int isNegative = 0, inNumber = 0;
  ssize_t bytesRead;

  while (1)
  {
    // Didn't read transition table file
    if ((bytesRead = read(*fd, buffer, BUFFER_SIZE)) < 0)
    {
      perror("Error reading transition table file");
      break;
    }

    // Read the transition table content
    // Note that we handle to handle negative sign, digit and white space
    for (int i = 0; i < bytesRead; i++)
    {
      char character = buffer[i];

      if (character == '-' && !inNumber)
      {
        isNegative = 1;
        inNumber = 1;
      }
      else if (character >= '0' && character <= '9')
      {
        num = num * 10 + (character - '0');
        inNumber = 1;
      }
      else if (character == ' ' || character == '\t')
      {
        if (inNumber)
        {
          arr[row][col] = isNegative ? -num : num;
          col++;

          num = 0;
          isNegative = 0;
          inNumber = 0;
        }
      }
      else if (character == '\n')
      {
        if (inNumber)
        {
          arr[row][col] = isNegative ? -num : num;
          col++;

          num = 0;
          isNegative = 0;
          inNumber = 0;
        }

        if (col > 0)
        {
          row++;
          col = 0;
        }
      }
    }

    if (bytesRead < BUFFER_SIZE)
    {
      break;
    }
  }

  // If file does not end with new line, add the last number
  if (inNumber)
  {
    arr[row][col] = isNegative ? -num : num;
    col++;
  }
}
