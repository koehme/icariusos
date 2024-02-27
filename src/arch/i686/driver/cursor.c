/**
 * @file cursor.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdint.h>

#include "cursor.h"
#include "vga.h"
#include "io.h"

// Sets the cursor to the specified screen position
void cursor_set(const uint8_t y, const uint8_t x)
{
    const uint16_t linear_position = (y * 80) + x;

    if (linear_position >= 0 && linear_position < (80 * 25))
    {
        // Set the low offset of the cursor
        asm_outb(VGA_CTRL, VGA_LOW_OFFSET);
        asm_outb(VGA_DATA, (uint8_t)(linear_position & 0xff));
        // Set the high offset of the cursor
        asm_outb(VGA_CTRL, VGA_HIGH_OFFSET);
        asm_outb(VGA_DATA, (uint8_t)((linear_position >> 8) & 0xff));
    };
    return;
};