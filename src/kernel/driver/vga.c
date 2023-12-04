/**
 * @file vga.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>

#include "vga.h"
#include "string.h"

VGADisplay vga_display;

static uint16_t make_ch(const uint8_t ch, const VGADisplayColor color)
{
    return ch | (uint16_t)(color << 8);
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
