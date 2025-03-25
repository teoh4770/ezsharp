
#include "string.h"

int _strlen(const char *src)
{
  size_t length = 0;

  while (src[length] != '\0')
  {
    length++;
  }
  return length;
}

int _strcmp(char *str1, char *str2)
{
  // Continue if both strings are non-null and characters are equal
  while (*str1 && (*str1 == *str2))
  {
    str1++;
    str2++;
  }

  // Return the difference between the first mismatched charcters
  return (unsigned char)*str1 - (unsigned char)*str2;
}

char *_strncpy(char *dest, const char *src, size_t n)
{
  if (dest == NULL || src == NULL || n == 0)
  {
    return dest;
  }

  size_t i = 0;

  // Copy characters from src to dest
  while (i < n && src[i] != '\0')
  {
    dest[i] = src[i];
    i++;
  }

  // Pad the rest with null terminators if needed
  while (i < n)
  {
    dest[i] = '\0';
    i++;
  }

  return dest;
}
