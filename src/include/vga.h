/**
 * @file vga.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VGA_H
#define VGA_H

#define VGA_DEBUG_DELAY 0

#include <stdint.h>

#include "cursor.h"
#include "icarius.h"

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
void vga_print(vga_t* self, const char* str, const uint8_t color);
void vga_print_ch(vga_t* self, const char ch, const uint8_t color);
void vga_display_set_cursor(vga_t* self, const uint8_t y, const uint8_t x);

#endif
