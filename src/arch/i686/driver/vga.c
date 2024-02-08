/**
 * @file vga.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>

#include "vga.h"
#include "mem.h"
#include "cursor.h"
#include "string.h"
#include "kernel.h"

VGADisplay vga_display = {
    .framebuffer = 0x0,
    .cursor_x = 0,
    .cursor_y = 0,
    .width = 0,
    .height = 0,
};

/**
 * @brief Create a VGA character with color attributes.
 * @param ch The ASCII character to be combined with color information.
 * @param color The VGAColor enum representing the foreground and background colors.
 * @return uint16_t A 16-bit value representing the combined character and color.
 */
static uint16_t make_ch(const uint8_t ch, const VGAColor color)
{
    return ch | (uint16_t)(color << 8);
};

/**
 * @brief Places the specified character at the given coordinates (y, x).
 * @param self A pointer to the VGADisplay structure representing the VGA display.
 * @param y The vertical position (row) where the character will be placed.
 * @param x The horizontal position (column) where the character will be placed.
 * @param ch The ASCII code of the character to be displayed.
 * @param color The color of the character on the VGA display.
 * @return void
 */
static void vga_display_put_ch_at(VGADisplay *self, const uint8_t y, const uint8_t x, const uint8_t ch, const VGAColor color)
{
    const uint16_t linear_address = (y * self->width) + x;
    self->framebuffer[linear_address] = make_ch(ch, color);
    return;
};

/**
 * @brief Clears a specified row in the VGA display by filling it with blank characters.
 * @param self A pointer to the VGADisplay structure.
 * @return void
 */
static void vga_clear_last_line(VGADisplay *self)
{
    uint16_t *last_line = (uint16_t *)self->framebuffer + (self->height - 1) * self->width;
    const uint16_t blank = make_ch(' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    mset16(last_line, blank, self->width);
    return;
};

/**
 * Scrolls the VGA display up by the specified number of lines.
 * @param self A pointer to the VGADisplay structure representing the display.
 * @param lines The number of lines by which to scroll up.
 * @return void
 */
static void vga_scroll_up(VGADisplay *self, const uint32_t lines)
{
    uint16_t *start_first_line = (uint16_t *)self->framebuffer;
    uint16_t *start_second_line = (uint16_t *)self->framebuffer + lines * self->width;
    const size_t bytes = self->width * (self->height - lines) * 2;
    mcpy(start_first_line, start_second_line, bytes);
    return;
};

/**
 * @brief Scrolls the content of a VGA display.
 * @param self A pointer to the VGADisplay structure.
 * @return void
 */
static void vga_scroll(VGADisplay *self)
{
    // Check if the cursor is beyond the visible areac
    if (self->cursor_y >= self->height)
    {
        vga_scroll_up(self, 1);
        vga_clear_last_line(self);
        self->cursor_y = self->height - 1;
    };
    return;
};

/**
 * @brief Initializes the VGA display by configuring the necessary parameters, such as the framebuffer, cursor position, width and height.
 * @param self A pointer to the VGADisplay structure representing the VGA display.
 * @param framebuffer A pointer to the  uint16_t array serving as the framebuffer.
 * @param width The width of the VGA display in pixels.
 * @param height The height of the VGA display in pixels.
 * @return void
 */
void vga_display_init(VGADisplay *self, uint16_t *framebuffer, const uint8_t width, const uint8_t height)
{
    self->framebuffer = framebuffer;
    self->cursor_x = 0;
    self->cursor_y = 0;
    self->width = width;
    self->height = height;
    return;
};

/**
 * @brief Writes a character to the VGA display at the current cursor position.
 * @param self   A pointer to the VGADisplay structure representing the VGA display instance.
 * @param ch     The character to be written to the display.
 * @param color  The color in which the character should be displayed.
 * @return void
 */
static void vga_display_write(VGADisplay *self, uint8_t ch, const VGAColor color)
{
    if (ch == '\n' || ch == '\r')
    {
        // Newline handler
        self->cursor_x = 0;
        self->cursor_y++;
    }
    else if (ch == '\b')
    {
        // Backspace handler move cursor one pos to the left, if a new column was added
        if (self->cursor_x > 0)
        {
            self->cursor_x--;
        }
        else if (self->cursor_y > 0)
        {
            // If at the beginning of the new line or row, move to the end of the previous line
            self->cursor_x = self->width - 1;
            self->cursor_y--;
        };
        vga_display_put_ch_at(self, self->cursor_y, self->cursor_x, ' ', color);
    }
    else
    {
        vga_display_put_ch_at(self, self->cursor_y, self->cursor_x, ch, color);
        self->cursor_x++;
    };

    if (self->cursor_x >= self->width)
    {
        self->cursor_x = 0;
        self->cursor_y++;
    };
    vga_scroll(self);
    return;
};

void vga_display_set_cursor(VGADisplay *self, const uint8_t y, const uint8_t x)
{
    self->cursor_x = x;
    self->cursor_y = y;
    return;
};

/**
 * Clears the content of the VGA display.
 * @param self A pointer to the VGADisplay structure representing the VGA display instance.
 * @return void
 */
void vga_display_clear(VGADisplay *self)
{
    for (size_t y = 0; y < self->height; y++)
    {
        for (size_t x = 0; x < self->width; x++)
        {
            vga_display_put_ch_at(self, y, x, ' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        };
    };
    return;
};

/**
 * @brief Prints a null-terminated string on the VGA display.
 * @param self A pointer to the VGADisplay structure representing the VGA display.
 * @param str A pointer to the \0-terminated string to be printed on the VGA display.
 * @return void
 */
void vga_print(VGADisplay *self, const char *str, const VGAColor color)
{
    const size_t len = slen(str);

    for (size_t i = 0; i < len; i++)
    {
        const char ch = str[i];
        kdelay(VGA_DEBUG_DELAY);
        vga_display_write(self, ch, color);
    };
    cursor_set(self->cursor_y, self->cursor_x);
    return;
};

void vga_print_ch(VGADisplay *self, const char ch, const VGAColor color)
{
    vga_display_write(self, ch, color);
    cursor_set(self->cursor_y, self->cursor_x);
    return;
};
