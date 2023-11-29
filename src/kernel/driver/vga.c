/**
 * @file vga.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>

#include "vga.h"

volatile uint16_t *vga_framebuffer;
uint16_t cursor_x;
uint16_t cursor_y;

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

void vga_display_init(volatile uint16_t *framebuffer)
{
    vga_framebuffer = framebuffer;
    cursor_x = 0;
    cursor_y = 0;
    return;
};

static void vga_display_put_ch_at(const uint8_t y, const uint8_t x, const uint8_t ch, const VGADisplayColor color)
{
    const uint16_t linear_address = (y * 80) + x;
    vga_framebuffer[linear_address] = make_ch(ch, color);
    return;
};

static void vga_display_write(uint8_t ch, const VGADisplayColor color)
{
    if (ch == '\n')
    {
        cursor_x = 0;
        cursor_y++;
        return;
    };
    vga_display_put_ch_at(cursor_y, cursor_x, ch, color);
    cursor_x++;

    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;
    };
    return;
};

void vga_display_clear(void)
{
    for (int y = 0; y < 25; y++)
    {
        for (int x = 0; x < 80; x++)
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
