/**
 * @file memory.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mem.h"

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *byte_ptr = (unsigned char *)ptr;
    unsigned char byte_value = (unsigned char)value;

    for (size_t i = 0; i < num; ++i)
    {
        byte_ptr[i] = byte_value;
    };
    return ptr;
};