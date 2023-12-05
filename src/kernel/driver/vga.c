/**
 * @file vga.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>

#include "vga.h"
#include "memory.h"
#include "string.h"

VGADisplay vga_display;

static uint16_t make_ch(const uint8_t ch, const VGADisplayColor color)
{
    return ch | (uint16_t)(color << 8);
};

/**
 * @brief Clears a specified row in the VGA display by filling it with blank characters.
 * @param self A pointer to the VGADisplay structure.
 * @return void
 */
static void vga_clear_last_line(VGADisplay *self)
{
    uint16_t *linear_clear_position = (uint16_t *volatile)self->framebuffer + (self->height - 1) * self->width * 2;
    const uint16_t blank = make_ch(' ', VGA_COLOR_BLACK);
    mset(linear_clear_position, blank, self->width * sizeof(uint16_t));
    return;
};

/**
 * @brief Scrolls up the content of a VGA display.
 * Scrolls up the content of the VGA display, starting from row 1,
 * effectively shifting everything up by one row. The specified number of rows
 * (`rows_to_scroll`) determines the extent of the scroll operation.
 * @param self A pointer to the VGADisplay structure.
 * @param rows The number of rows to scroll up.
 * @return void
 */
static void vga_scroll_up(VGADisplay *self, unsigned int rows)
{
    uint16_t *dest = (uint16_t *volatile)self->framebuffer;
    uint16_t *src = (uint16_t *volatile)self->framebuffer + rows * self->width;
    const size_t bytes = (self->height - rows) * self->width * 2;
    mcpy(dest, src, bytes);
    return;
};

/**
 * @brief Scrolls the content of a VGA display.
 * Checks if the cursor is beyond the visible area and if so, it scrolls up the content, clears the last row and updates the cursor position.
 * @param self A pointer to the VGADisplay structure.
 * @return void
 */
static void vga_scroll(VGADisplay *self)
{
    // Check if the cursor is beyond the visible area
    if (self->cursor_y >= self->height)
    {
        // Scroll up the content
        vga_scroll_up(self, 1);
        // Clear the last row
        vga_clear_last_line(self);
        // Update the cursor position
        self->cursor_y = self->height - 1;
    };
    return;
};

void vga_display_init(VGADisplay *self, volatile uint16_t *framebuffer, const uint8_t width, const uint8_t height)
{
    self->framebuffer = framebuffer;
    self->cursor_x = 0;
    self->cursor_y = 0;
    self->width = width;
    self->height = height;
    return;
};

static void vga_display_put_ch_at(VGADisplay *self, const uint8_t y, const uint8_t x, const uint8_t ch, const VGADisplayColor color)
{
    const uint16_t linear_address = (y * self->width) + x;
    self->framebuffer[linear_address] = make_ch(ch, color);
    return;
};

static void vga_display_write(VGADisplay *self, uint8_t ch, const VGADisplayColor color)
{
    if (ch == '\n')
    {
        self->cursor_x = 0;
        self->cursor_y++;
        return;
    };
    vga_display_put_ch_at(self, self->cursor_y, self->cursor_x, ch, color);
    self->cursor_x++;

    if (self->cursor_x >= self->width)
    {
        self->cursor_x = 0;
        self->cursor_y++;
    };
    vga_scroll(self);
    return;
};

void vga_display_clear(VGADisplay *self)
{
    for (int y = 0; y < self->height; y++)
    {
        for (int x = 0; x < self->width; x++)
        {
            vga_display_put_ch_at(self, y, x, ' ', VGA_COLOR_BLACK);
        };
    };
    return;
};

void vga_print(VGADisplay *self, const char *str)
{
    const size_t len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        const char ch = str[i];
        vga_display_write(self, ch, VGA_COLOR_CYAN);
    };
    return;
};
