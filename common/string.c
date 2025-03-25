
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

int _strcmp(const char *str1, const char *str2)
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

int _strncmp(char *str1, const char *str2, size_t n)
{
  // Compare up to n characters or until a null terminator is found
  while (n > 0 && *str1 && (*str1 == *str2))
  {
    str1++;
    str2++;
    n--;
  }

  // If n reaches 0, it means the first n characters are identical
  if (n == 0)
  {
    return 0;
  }

  // Return the difference between the first mismatched characters
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
