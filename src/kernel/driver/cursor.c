/**
 * @file cursor.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdint.h>

#include "cursor.h"
#include "vga.h"
#include "io.h"

/**
 * @brief Sets the text cursor position in VGA text mode.
 * @param y The new vertical position (row) of the text cursor.
 * @param x The new horizontal position (column) of the text cursor.
 */
void cursor_set(const uint8_t y, const uint8_t x)
{
    asm_outb(VGA_CTRL, VGA_LOW_OFFSET);
    asm_outb(VGA_DATA, y);

    asm_outb(VGA_CTRL, VGA_HIGH_OFFSET);
    asm_outb(VGA_DATA, x);
    return;
};