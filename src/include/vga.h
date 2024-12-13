/**
 * @file vga.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VGA_H
#define VGA_H

#define VGA_DEBUG_DELAY 0

#include "cursor.h"
#include <stdint.h>

#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5
#define VGA_LOW_OFFSET 0x0F
#define VGA_HIGH_OFFSET 0x0E

typedef enum vga_color {
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
} vga_color_t;

typedef struct vga {
	uint16_t* framebuffer;
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint8_t width;
	uint8_t height;
} vga_t;

extern vga_t vga_display;

void vga_display_init(vga_t* self, uint16_t* framebuffer, const uint8_t width, const uint8_t height);
void vga_display_clear(vga_t* self);
void vga_print(vga_t* self, const char* str, const vga_color_t color);
void vga_print_ch(vga_t* self, const char ch, const vga_color_t color);
void vga_display_set_cursor(vga_t* self, const uint8_t y, const uint8_t x);

#endif
