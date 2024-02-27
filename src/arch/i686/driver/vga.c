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

// Create a VGA character with color attributes
static uint16_t make_ch(const uint8_t ch, const VGAColor color)
{
    return ch | (uint16_t)(color << 8);
};

// Places the specified character at the given coordinates (y, x)
static void vga_display_put_ch_at(VGADisplay *self, const uint8_t y, const uint8_t x, const uint8_t ch, const VGAColor color)
{
    const uint16_t linear_address = (y * self->width) + x;
    self->framebuffer[linear_address] = make_ch(ch, color);
    return;
};

// Flushing the last line
static void vga_clear_last_line(VGADisplay *self)
{
    uint16_t *last_line = (uint16_t *)self->framebuffer + (self->height - 1) * self->width;
    const uint16_t blank = make_ch(' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    mset16(last_line, blank, self->width);
    return;
};

// Scroll up the display by moving content from the second line to the first
static void vga_scroll_up(VGADisplay *self, const uint32_t lines)
{
    uint16_t *start_first_line = (uint16_t *)self->framebuffer;
    uint16_t *start_second_line = (uint16_t *)self->framebuffer + lines * self->width;
    const size_t bytes = self->width * (self->height - lines) * 2;
    mcpy(start_first_line, start_second_line, bytes);
    return;
};

// Scrolls the content and adjusts cursor position if the cursor_y boundary is reached
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

// Initializes the display by configuring the necessary parameters, such as the framebuffer, cursor position, width and height
void vga_display_init(VGADisplay *self, uint16_t *framebuffer, const uint8_t width, const uint8_t height)
{
    self->framebuffer = framebuffer;
    self->cursor_x = 0;
    self->cursor_y = 0;
    self->width = width;
    self->height = height;
    return;
};

// Writes a character to the display at the current cursor position
static void vga_display_write(VGADisplay *self, uint8_t ch, const VGAColor color)
{
    switch (ch)
    {
    case '\n':
    case '\r':
    {
        // Newline handler
        self->cursor_x = 0;
        self->cursor_y++;
        break;
    };
    case '\b':
    {
        // Backspace handler
        if (self->cursor_x > 0)
        {
            // Move cursor one position to the left
            self->cursor_x--;
        }
        // If at the beginning of the line, move to the end of the previous line
        else if (self->cursor_y > 0)
        {
            self->cursor_x = self->width - 1;
            self->cursor_y--;
        };
        // Replace deleted character with a space
        vga_display_put_ch_at(self, self->cursor_y, self->cursor_x, ' ', color);
        break;
    };
    default:
    {
        // Write character to display
        vga_display_put_ch_at(self, self->cursor_y, self->cursor_x, ch, color);
        self->cursor_x++;
        break;
    };
    };
    // Check for end of line
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

// Clears the content of the display
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

// Prints a null-terminated string on the display
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
// Prints a single char on the display
void vga_print_ch(VGADisplay *self, const char ch, const VGAColor color)
{
    vga_display_write(self, ch, color);
    cursor_set(self->cursor_y, self->cursor_x);
    return;
};
