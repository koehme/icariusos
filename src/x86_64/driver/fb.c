/**
 * @file fb.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fb.h"

static fb_t fb = {};

/* EXTERNAL API */

/* PUBLIC API */
void fb_setup(void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp, const fb_color_format_t format);
void fb_clear(const uint32_t color);

/* INTERNAL API */
void fb_setup(void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp, const fb_color_format_t format)
{
	fb.frontbuffer = (uint32_t*)addr;
	fb.width = width;
	fb.height = height;
	fb.pitch = pitch;
	fb.bpp = bpp;
	fb.buffer_size = height * pitch;
	fb.format = format;
	// TODO => DOUBLE BUFFERING
	fb.backbuffer = NULL;
	return;
};

void fb_clear(const uint32_t color)
{
	if (!fb.frontbuffer) {
		return;
	};
	const uint32_t pixel_per_line = fb.pitch / (fb.bpp / 8);

	for (uint32_t y = 0; y < fb.height; y++) {
		for (uint32_t x = 0; x < fb.width; x++) {
			fb.frontbuffer[y * pixel_per_line + x] = color;
		};
	};
	return;
};