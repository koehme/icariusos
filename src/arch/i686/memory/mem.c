/**
 * @file mem.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mem.h"

// Set a memory block with a 8-bit value repeated throughout for the specified number of bytes
void *mset8(void *dest, const uint8_t value, size_t n_bytes)
{
    uint8_t *buffer = (uint8_t *)dest;

    for (; n_bytes != 0; n_bytes--)
    {
        *buffer = value;
        buffer++;
    };
    return dest;
};

// Set a memory block with a 16-bit value repeated throughout for the specified number of bytes
void *mset16(void *dest, const uint16_t value, size_t n_bytes)
{
    uint16_t *buffer = (uint16_t *)dest;

    for (; n_bytes != 0; n_bytes--)
    {
        *buffer = value;
        buffer++;
    };
    return dest;
};

// Copy a memory block from source to destination with specified number of bytes
void *mcpy(void *dest, const void *src, size_t n_bytes)
{
    uint8_t *byte_dest = (uint8_t *)dest;
    const uint8_t *src_ptr = (const uint8_t *)src;

    while (n_bytes)
    {
        *byte_dest = *src_ptr;
        byte_dest++;
        src_ptr++;
        n_bytes--;
    };
    return dest;
};

// Compare two memory blocks byte by byte and return the difference. If all bytes are equal, returns 0
int32_t mcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    while (n > 0)
    {
        if (*p1 != *p2)
        {
            return (int32_t)(*p1) - (int32_t)(*p2);
        };
        p1++;
        p2++;
        n--;
    };
    return 0;
};