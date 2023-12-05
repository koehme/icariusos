/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "string.h"

size_t strlen(const char *str)
{
    size_t i = 0;

    for (;;)
    {
        if (str[i] == '\0')
        {
            break;
        };
        i++;
    };
    return i;
};
