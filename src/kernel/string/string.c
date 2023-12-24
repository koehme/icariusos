/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "string.h"

bool is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
};

/**
 * @brief Calculate the length of a null-terminated C string.
 * This function takes a pointer to a null-terminated C string and iterates through
 * each character until the null terminator '\0' is encountered. It then returns
 * the number of characters in the string, excluding the null terminator.
 * @param str A pointer to the null-terminated C string whose length is to be determined.
 * @return size_t The length of the C string, excluding the null terminator.
 * @note The function assumes that the input string is null-terminated. If the input
 * string is not null-terminated, the behavior is undefined.
 * @note This function has a time complexity of O(n), where n is the length of the input string.
 */
size_t slen(const char *str)
{
    size_t i;

    for (i = 0; str[i] != '\0'; ++i)
    {
    };
    return i;
};