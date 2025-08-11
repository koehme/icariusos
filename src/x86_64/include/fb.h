/**
 * @file fb.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "icarius.h"
#include <stddef.h>
#include <stdint.h>

typedef struct fb_info {
	void* vaddr; // Start address of the framebuffer in memory
	uint32_t width;
	uint32_t height;
	uint32_t pitch; // Number of bytes per scanline (may include padding)
	uint8_t bpp;	// Bits per pixel (e.g., 32 for ARGB8888)
	struct {
		uint8_t r_shift, g_shift, b_shift, a_shift;
		uint8_t r_size, g_size, b_size, a_size;
	} fmt; // Describes how color data is stored in each pixel
} fb_info_t;

typedef struct fb {
	uint32_t* frontbuffer; // Points to the actual framebuffer in video memory mmio-mapped (what is visible on screen)
	uint32_t* backbuffer;  // Optional off-screen buffer for double buffering
	fb_info_t info;	       // Framebuffer layout and pixel format details provided by the bootloader
} fb_t;

void fb_setup(const fb_info_t* info);
void fb_clear(const uint32_t color);
void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);
void fb_scroll(const uint32_t color, const uint32_t rows);
uint32_t fb_pack_rgba(const fb_info_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);