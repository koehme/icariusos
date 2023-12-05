/**
 * @file memory.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "memory.h"

void *mset(void *dest, const int value, const size_t n_bytes)
{
    unsigned char *byte_dest = (unsigned char *)dest;
    unsigned char byte_value = (unsigned char)value;

    for (size_t i = 0; i < n_bytes; ++i)
    {
        byte_dest[i] = byte_value;
    };
    return dest;
};

void *mcpy(void *dest, const void *src, size_t n_bytes)
{
    unsigned char *byte_dest = (unsigned char *)dest;
    const unsigned char *byte_src = (const unsigned char *)src;

    while (n_bytes)
    {
        *byte_dest = *byte_src;
        byte_dest++;
        byte_src++;
        n_bytes--;
    };
    return dest;
};