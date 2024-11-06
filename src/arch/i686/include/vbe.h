/**
 * @file vbe.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VBE_H
#define VBE_H

#include <stdint.h>

#define VBE_DEBUG_DELAY 0
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

const static uint8_t ascii_bitmap[128][CHAR_HEIGHT] = {
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    // 0x41 ('A')
    {
	0b00111100,
	0b01000010,
	0b01000010,
	0b01000010,
	0b01111110,
	0b01000010,
	0b01000010,
	0b01000010,
    },
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},

};

typedef struct VBEDisplay {
	void* addr;	 // where the pixel data starts
	uint32_t width;	 // horizontal resolution
	uint32_t height; // vertical resolution
	uint32_t pitch;	 // Number of bytes between the start of one row and the start of the next (often called "stride")
	uint32_t bpp;	 // Bits per pixel (color depth, e.g., 32 for 32-bit color)
} VBEDisplay;

typedef enum VBEColor {
	VBE_COLOR_BLACK = 0x000000,
	VBE_COLOR_RED = 0xFF0000,
	VBE_COLOR_GREEN = 0x00FF00,
	VBE_COLOR_BLUE = 0x0000FF,
	VBE_COLOR_WHITE = 0xFFFFFF,
	VBE_COLOR_YELLOW = 0xFFFF00,
} VBEColor;

extern VBEDisplay vbe_display;

void vbe_init(VBEDisplay* self, const void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp);
void vbe_put_pixel_at(VBEDisplay* self, const uint32_t x, const uint32_t y, const VBEColor color);
void vbe_draw_horizontal_line(VBEDisplay* self, const uint32_t y, const VBEColor color);
void vbe_draw_char(VBEDisplay* self, const uint32_t x, const uint32_t y, const char ch, const VBEColor color);

#endif