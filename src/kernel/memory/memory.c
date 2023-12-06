/**
 * @file memory.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "memory.h"
#include <stdint.h>

/**
 * @brief Sets a block of memory to the specified value.
 * @param dest Pointer to the memory location.
 * @param value The value to set (as an integer).
 * @param n_bytes Number of bytes to set.
 * @return Pointer to the memory location.
 */
void *msetb(void *dest, const uint8_t value, size_t n_bytes)
{
    uint8_t *temp = (uint8_t *)dest;

    for (; n_bytes != 0; n_bytes--)
    {
        *temp = value;
        temp++;
    };
    return dest;
};

void *msetw(void *dest, const uint16_t value, size_t n_bytes)
{
    uint16_t *temp = (uint16_t *)dest;

    for (; n_bytes != 0; n_bytes--)
    {
        *temp = value;
        temp++;
    };
    return dest;
}

/**
 * @brief Copies a block of memory from the source location to the destination location.
 * @param dest Pointer to the destination memory location.
 * @param src Pointer to the source memory location.
 * @param n_bytes Number of bytes to copy.
 * @return Pointer to the destination memory location.
 */
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