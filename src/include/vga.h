/**
 * @file vga.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VGA_H
#define VGA_H

#include <stdint.h>

typedef enum VGAColor
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GRAY = 7,
    VGA_COLOR_DARK_GRAY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
} VGAColor;

typedef struct VGADisplay
{
    volatile uint16_t *framebuffer;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t width;
    uint8_t height;
} VGADisplay;

extern VGADisplay vga_display;

void vga_display_init(VGADisplay *self, volatile uint16_t *framebuffer, const uint8_t width, const uint8_t height);
void vga_display_clear(VGADisplay *self);
void vga_print(VGADisplay *self, const char *str);

#endif
