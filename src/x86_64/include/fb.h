/**
 * @file fb.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "icarius.h"
#include <stddef.h>
#include <stdint.h>

enum {
	FB_CHANNEL_RED = 0,
	FB_CHANNEL_GREEN = 1,
	FB_CHANNEL_BLUE = 2,
	FB_CHANNEL_ALPHA = 3,
};

typedef struct fb_color_format {
	uint8_t r_shift, g_shift, b_shift, a_shift;
	uint8_t r_size, g_size, b_size, a_size;
} fb_color_format_t;

typedef struct fb {
	uint32_t* frontbuffer;	  // Points to the actual framebuffer in video memory mmio-mapped (what is visible on screen)
	uint32_t* backbuffer;	  // Optional off-screen buffer for double buffering
	uint32_t width;		  // Horizontal resolution in pixels
	uint32_t height;	  // Vertical resolution in pixels
	uint32_t pitch;		  // Number of bytes between rows (also called stride)
	uint32_t bpp;		  // Bits per pixel (e.g., 32 for 0xAARRGGBB)
	size_t buffer_size;	  // Total size of one framebuffer in bytes (height * pitch)
	fb_color_format_t format; // Describes how to assemble a pixel from RGBA components
	uint32_t cursor_x;
	uint32_t cursor_y;
} fb_t;

void fb_setup(void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp, const fb_color_format_t format);
void fb_clear(const uint32_t color);
void fb_scroll(const uint32_t bg);
void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);