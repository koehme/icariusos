/**
 * @file vga.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>

#include "vga.h"

VGADisplay vga_display;

static size_t strlen(const char *str)
{
    size_t i = 0;

    for (;;)
    {
        if (str[i] == '\0')
        {
            break;
        };
        i++;
    };
    return i;
};

static uint16_t make_ch(const uint8_t ch, const VGADisplayColor color)
{
    return ch | (uint16_t)(color << 8);
};

void vga_display_init(volatile uint16_t *framebuffer, const uint8_t width, const uint8_t height)
{
    vga_display.framebuffer = framebuffer;
    vga_display.cursor_x = 0;
    vga_display.cursor_y = 0;
    vga_display.width = width;
    vga_display.height = height;
    return;
};

static void vga_display_put_ch_at(const uint8_t y, const uint8_t x, const uint8_t ch, const VGADisplayColor color)
{
    const uint16_t linear_address = (y * vga_display.width) + x;
    vga_display.framebuffer[linear_address] = make_ch(ch, color);
    return;
};

static void vga_display_write(uint8_t ch, const VGADisplayColor color)
{
    if (ch == '\n')
    {
        vga_display.cursor_x = 0;
        vga_display.cursor_y++;
        return;
    };
    vga_display_put_ch_at(vga_display.cursor_y, vga_display.cursor_x, ch, color);
    vga_display.cursor_x++;

    if (vga_display.cursor_x >= vga_display.width)
    {
        vga_display.cursor_x = 0;
        vga_display.cursor_y++;
    };
    return;
};

void vga_display_clear(void)
{
    for (int y = 0; y < vga_display.height; y++)
    {
        for (int x = 0; x < vga_display.width; x++)
        {
            vga_display_put_ch_at(y, x, ' ', VGA_COLOR_BLACK);
        };
    };
    return;
};

void vga_print(const char *str)
{
    const size_t len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        const char ch = str[i];
        vga_display_write(ch, VGA_COLOR_CYAN);
    };
    return;
};
