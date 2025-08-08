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
void fb_setup(void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp, const fb_color_format_t format);
void fb_clear(const uint32_t color);
void fb_draw_string(const char* str, const uint32_t color);

/* INTERNAL API */
static void _scroll(const uint32_t bg);
static void _put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);
static void _draw_ch(const char ch, const uint32_t color);

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
	fb.cursor_x = 0;
	fb.cursor_y = 0;
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
	fb.cursor_x = 0;
	fb.cursor_y = 0;
	return;
};

void _put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color)
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

void _draw_ch(const char ch, const uint32_t color)
{
	if (!fb.frontbuffer) {
		errno = ENODEV;
		return;
	};
	// Out of ASCII bounds
	if ((uint8_t)ch >= 128) {
		errno = EINVAL;
		return;
	};
	const uint8_t* glyph = ascii_bitmap[(uint8_t)ch];

	switch (ch) {
	case '\n': {
		// Newline: Move cursor to the beginning of the next line
		fb.cursor_x = 0;
		fb.cursor_y += FONT_HEIGHT;
		break;
	};
	case '\r': {
		// Carriage Return: Move cursor to the beginning of the current line
		fb.cursor_x = 0;
		break;
	};
	case '\b': {
		// Backspace: Move cursor back and erase the previous character
		if (fb.cursor_x >= FONT_WIDTH) {
			fb.cursor_x -= FONT_WIDTH;
		} else if (fb.cursor_y >= FONT_HEIGHT) {
			fb.cursor_y -= FONT_HEIGHT;
			fb.cursor_x = fb.width - FONT_WIDTH;
		};
		// Erase the previous character by clearing its pixels
		_put_pixel_at(fb.cursor_x, fb.cursor_y, BLACK);
		break;
	};
	default: {
		// Iterate over each row of the character bitmap
		for (size_t row = 0; row < FONT_HEIGHT; row++) {
			const uint8_t row_bits = glyph[row];
			// Iterate over each column of the character bitmap
			for (size_t col = 0; col < FONT_WIDTH; col++) {
				// Check if the pixel should be turned on
				const bool is_on = row_bits & (1 << col);
				// Check if the pixel should be turned on

				if (is_on) {
					// Draw the pixel at the calculated position with the specified color
					_put_pixel_at(fb.cursor_x + col, fb.cursor_y + row, color);
				};
			};
		};
		// Update the cursor position to the right after drawing the character
		fb.cursor_x += FONT_WIDTH;
		// Handle line wrapping if the cursor exceeds the display width
		if (fb.cursor_x + FONT_WIDTH > fb.width) {
			fb.cursor_x = 0;
			fb.cursor_y += FONT_HEIGHT;
		};
		break;
	};
	};
	// Check if the cursor has moved beyond the display height and perform scrolling if necessary
	if (fb.cursor_y + FONT_HEIGHT > fb.height) {
		// Scroll the display content upwards and clear the bottom area
		_scroll(BLACK);
	};
	return;
};

static void _scroll(const uint32_t bg)
{
	const uint32_t scroll_height = FONT_HEIGHT;
	const uint32_t scroll_bytes = scroll_height * fb.pitch;

	uint8_t* src = (uint8_t*)fb.frontbuffer + scroll_bytes;
	uint8_t* dest = (uint8_t*)fb.frontbuffer;

	const uint32_t total_bytes = (fb.height - scroll_height) * fb.pitch;

	memmove(dest, src, total_bytes);

	for (uint32_t y = fb.height - scroll_height; y < fb.height; y++) {
		for (uint32_t x = 0; x < fb.width; x++) {
			_put_pixel_at(x, y, bg);
		};
	};
	fb.cursor_y -= scroll_height;
	return;
};

void fb_draw_string(const char* str, const uint32_t color)
{
	while (*str) {
		_draw_ch(*str, color);
		str++;
	};
	return;
};