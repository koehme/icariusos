/**
 * @file fb.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "errno.h"
#include "fb.h"
#include "string.h"

static fb_t fb = {};

/* EXTERNAL API */
extern int errno;

/* PUBLIC API */
fb_t* fb_get(void);
void fb_setup(void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp, const fb_color_format_t format);
void fb_clear(const uint32_t color);
void fb_scroll(const uint32_t bg);
void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);

/* INTERNAL API */
// --

fb_t* fb_get(void) { return &fb; };

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
		errno = ENODEV;
		return;
	};

	for (uint32_t y = 0; y < fb.height; y++) {
		uint32_t* row = (uint32_t*)(((uint8_t*)fb.frontbuffer) + y * fb.pitch);

		for (uint32_t x = 0; x < fb.width; x++) {
			row[x] = color;
		};
	};
	return;
};

void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (!fb.frontbuffer) {
		errno = ENODEV;
		return;
	};
	// Check if the coordinates are within the display's resolution boundaries
	if (x >= fb.width || y >= fb.height) {
		errno = EINVAL;
		return;
	};
	uint32_t* row = (uint32_t*)(((uint8_t*)fb.frontbuffer) + y * fb.pitch);
	row[x] = color;
	return;
};

void fb_scroll(const uint32_t bg)
{
	const uint32_t scroll_bytes = FONT_HEIGHT * fb.pitch;
	uint8_t* src = (uint8_t*)fb.frontbuffer + scroll_bytes;
	uint8_t* dest = (uint8_t*)fb.frontbuffer;
	const uint32_t total_bytes = (fb.height - FONT_HEIGHT) * fb.pitch;

	memmove(dest, src, total_bytes);

	for (uint32_t y = fb.height - FONT_HEIGHT; y < fb.height; y++) {
		for (uint32_t x = 0; x < fb.width; x++) {
			fb_put_pixel_at(x, y, bg);
		};
	};
	return;
};