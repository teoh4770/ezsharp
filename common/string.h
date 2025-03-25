#ifndef STRING_H
#define STRING_H

#include <stdio.h>

// Recreate string functions

/**
 * Calculate the length of a given string.
 *
 * @param src Pointer to the input string (null-terminated).
 * @return The number of characters in the string, excluding the null terminator.
 */
int _strlen(const char *src);

/**
 * Compare two strings lexicographically.
 *
 * @param str1 Pointer to the first string (null-terminated).
 * @param str2 Pointer to the second string (null-terminated).
 * @return
 *      0 if both strings are equal,
 *      A negative value if str1 is lexicographically less than str2,
 *      A positive value if str1 is lexicographically greater than str2.
 */
int _strcmp(const char *str1, const char *str2);

/**
 * Compare up to n characters of two strings lexicographically.
 *
 * @param str1 Pointer to the first string (null-terminated).
 * @param str2 Pointer to the second string (null-terminated).
 * @param n Maximum number of characters to compare.
 * @return
 *      0 if both strings are equal within the first n characters,
 *      A negative value if str1 is lexicographically less than str2,
 *      A positive value if str1 is lexicographically greater than str2.
 */
int _strncmp(char *str1, const char *str2, size_t n);

/**
 * Copy up to n characters from the source string to the destination string.
 *
 * @param dest Pointer to the destination buffer.
 * @param src Pointer to the source string (null-terminated).
 * @param n Maximum number of characters to copy.
 * @return Pointer to the destination buffer (dest).
 *
 * Note: If the length of the source string is less than n, the remaining
 * characters in dest will be null-terminated. If src has more than n characters,
 * no null terminator will be added beyond n.
 */
char *_strncpy(char *dest, const char *src, size_t n);

#endif