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

VGADisplay vga_display;

/**
 * @brief Create a VGA character with color attributes.
 * Combines a given ASCII character with VGA color information
 * to produce a 16-bit value representing a character with associated color.
 * @param ch The ASCII character to be combined with color information.
 * @param color The VGAColor enum representing the foreground and background colors.
 * @return uint16_t A 16-bit value representing the combined character and color.
 */
static uint16_t make_ch(const uint8_t ch, const VGAColor color)
{
    return ch | (uint16_t)(color << 8);
};

/**
 * @brief Places the specified character at the given coordinates (y, x)
 * on the VGA display, using the provided color. It calculates the linear address
 * in the framebuffer and updates the corresponding memory location.
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
    uint16_t *last_line = (uint16_t *volatile)self->framebuffer + (self->height - 1) * self->width;
    const uint16_t blank = make_ch(' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    mset16(last_line, blank, self->width);
    return;
};

/**
 * Scrolls the VGA display up by the specified number of lines.
 * Performs a vertical scroll on the VGA display, moving the content
 * up by the specified number of lines. It is particularly useful when updating
 * the display content and needing to make room for new information.
 * @param self A pointer to the VGADisplay structure representing the display.
 * @param lines The number of lines by which to scroll up.
 * @return void
 */
static void vga_scroll_up(VGADisplay *self, const unsigned int lines)
{
    uint16_t *start_first_line = (uint16_t *volatile)self->framebuffer;
    uint16_t *start_second_line = (uint16_t *volatile)self->framebuffer + lines * self->width;
    const size_t bytes = self->width * (self->height - lines) * 2;
    mcpy(start_first_line, start_second_line, bytes);
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
 * @brief Initializes the VGA display by configuring the necessary parameters,
 * such as the framebuffer, cursor position, width and height.
 *
 * @param self A pointer to the VGADisplay structure representing the VGA display.
 * @param framebuffer A pointer to the volatile uint16_t array serving as the framebuffer.
 * @param width The width of the VGA display in pixels.
 * @param height The height of the VGA display in pixels.
 * @return void
 */
void vga_display_init(VGADisplay *self, volatile uint16_t *framebuffer, const uint8_t width, const uint8_t height)
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
 * If the character is a newline ('\n'), the cursor is moved to the beginning of the
 * next line. The character is displayed with the specified color.
 * @param self   A pointer to the VGADisplay structure representing the VGA display instance.
 * @param ch     The character to be written to the display.
 * @param color  The color in which the character should be displayed.
 * @return void
 */
static void vga_display_write(VGADisplay *self, uint8_t ch, const VGAColor color)
{
    if (ch == '\n')
    {
        self->cursor_x = 0;
        self->cursor_y++;
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

/**
 * Clears the content of the VGA display.
 * Iterates through each pixel on the VGA display and sets the character
 * to a space (' ') with a specified color (VGA_COLOR_BLACK). It effectively clears
 * the entire display, providing a blank canvas for subsequent rendering.
 * @param self A pointer to the VGADisplay structure representing the VGA display instance.
 * @return void
 */
void vga_display_clear(VGADisplay *self)
{
    for (int y = 0; y < self->height; y++)
    {
        for (int x = 0; x < self->width; x++)
        {
            vga_display_put_ch_at(self, y, x, ' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        };
    };
    return;
};

/**
 * @brief Prints a null-terminated string on the VGA display.
 * Iterates through the characters in the given null-terminated string
 * and writes each character to the VGA display using the specified color.
 * @param self A pointer to the VGADisplay structure representing the VGA display.
 * @param str A pointer to the null-terminated string to be printed on the VGA display.
 * @return void
 */
void vga_print(VGADisplay *self, const char *str)
{
    const size_t len = slen(str);

    for (int i = 0; i < len; i++)
    {
        const char ch = str[i];
        vga_display_write(self, ch, VGA_COLOR_LIGHT_GREEN);
    };
    cursor_set(self->cursor_y, self->cursor_x);
    return;
};
