/**
 * @file mem.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mem.h"

/**
 * @brief Sets a block of memory to the specified value.
 * @param dest  A pointer to the destination memory block.
 * @param value The 8-bit value to be set in each element of the memory block.
 * @param n_bytes The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
 */
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

/**
 * Sets each 16-bit element in the memory block pointed to by 'dest'
 * to the specified 'value'. The number of bytes processed is determined by the
 * 'n_bytes' parameter. The function returns a pointer to the destination memory block.
 * @param dest     A pointer to the destination memory block.
 * @param value    The 16-bit value to be set in each element of the memory block.
 * @param n_bytes  The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
 */
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

/**
 * @brief Copies a block of memory from the source location to the destination location.
 * @param dest     A pointer to the destination memory block.
 * @param src A Pointer to the source memory location.
 * @param n_bytes  The number of bytes to process, indicating the size of the memory block.
 * @return A pointer to the destination memory block ('dest').
 */
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
}