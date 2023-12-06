/**
 * @file memory.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "memory.h"
#include <stdint.h>

/**
 * @brief Sets a block of memory to the specified value.
 * @param dest  A pointer to the destination memory block.
 * @param value The 8-bit value to be set in each element of the memory block.
 * @param n_bytes The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
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

/**
 * Sets each 16-bit element in the memory block pointed to by 'dest'
 * to the specified 'value'. The number of bytes processed is determined by the
 * 'n_bytes' parameter. The function returns a pointer to the destination memory block.
 * @param dest     A pointer to the destination memory block.
 * @param value    The 16-bit value to be set in each element of the memory block.
 * @param n_bytes  The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
 */
void *msetw(void *dest, const uint16_t value, size_t n_bytes)
{
    uint16_t *temp = (uint16_t *)dest;

    for (; n_bytes != 0; n_bytes--)
    {
        *temp = value;
        temp++;
    };
    return dest;
};

/**
 * @brief Copies a block of memory from the source location to the destination location.
 * @param dest     A pointer to the destination memory block.
 * @param src A Pointer to the source memory location.
 * @param n_bytes  The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
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