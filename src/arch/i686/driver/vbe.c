/**
 * @file vbe.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vbe.h"
#include "string.h"

#include <stdbool.h>
#include <stddef.h>

VBEDisplay vbe_display = {
    .addr = 0x0,
    .width = 0,
    .height = 0,
    .pitch = 0,
    .bpp = 0,
    .cursor_x = 0,
    .cursor_y = 0,
};

void vbe_init(VBEDisplay* self, const void* addr, const uint32_t width, const uint32_t height, const uint32_t pitch, const uint32_t bpp)
{
	self->addr = (void*)addr;
	self->width = width;
	self->height = height;
	self->pitch = pitch;
	self->bpp = bpp;
	return;
};

static inline uint32_t calculate_pixel_offset(const uint32_t x, const uint32_t y, const uint32_t pitch, const uint32_t bytes_per_pixel)
{
	return y * pitch + x * bytes_per_pixel;
};

static void vbe_put_pixel_at(VBEDisplay* self, const uint32_t x, const uint32_t y, const VBEColor color)
{
	// Check if the coordinates are within the display's resolution boundaries
	if (x >= self->width || y >= self->height) {
		return;
	};
	const uint32_t bits_per_byte = 8;
	// Display color depth is given in bits-per-pixel (bpp), so dividing by 8 converts it to bytes-per-pixel
	const uint32_t bytes_per_pixel = self->bpp / bits_per_byte;
	const uint32_t pixel_offset = calculate_pixel_offset(x, y, self->pitch, bytes_per_pixel);
	const uint8_t* pixel_address = (uint8_t*)self->addr + pixel_offset;
	*((uint32_t*)pixel_address) = color;
	return;
};

void vbe_draw_hline(VBEDisplay* self, const uint32_t y, const VBEColor color)
{
	if (y >= self->height) {
		return;
	};

	for (size_t x = 0; x < self->width; x++) {
		vbe_put_pixel_at(self, x, y, color);
	};
	return;
};

static inline void vbe_clear_ch(VBEDisplay* self, const uint32_t x, const uint32_t y, const VBEColor color)
{
	for (size_t row = 0; row < FONT_HEIGHT; row++) {
		for (size_t col = 0; col < FONT_WIDTH; col++) {
			vbe_put_pixel_at(self, x + col, y + row, color);
		};
	};
	return;
};

static void vbe_scroll(VBEDisplay* self, const VBEColor background_color)
{
	const uint32_t bytes_per_pixel = self->bpp / 8;
	const uint32_t scroll_height = FONT_HEIGHT;
	const uint32_t scroll_bytes = scroll_height * self->pitch;

	uint8_t* src = (uint8_t*)self->addr + scroll_bytes;
	uint8_t* dest = (uint8_t*)self->addr;

	uint32_t total_bytes = (self->height - scroll_height) * self->pitch;

	mmove(dest, src, total_bytes);

	for (uint32_t y = self->height - scroll_height; y < self->height; y++) {
		for (uint32_t x = 0; x < self->width; x++) {
			vbe_put_pixel_at(self, x, y, background_color);
		};
	};
	self->cursor_y -= scroll_height;
	return;
};

void vbe_draw_ch(VBEDisplay* self, char ch, const VBEColor color)
{
	switch (ch) {
	case '\n': {
		// Newline: Move cursor to the beginning of the next line
		self->cursor_x = 0;
		self->cursor_y += FONT_HEIGHT;
		break;
	};
	case '\r': {
		// Carriage Return: Move cursor to the beginning of the current line
		self->cursor_x = 0;
		break;
	};
	case '\b': {
		// Backspace: Move cursor back and erase the previous character
		if (self->cursor_x >= FONT_WIDTH) {
			self->cursor_x -= FONT_WIDTH;
		} else if (self->cursor_y >= FONT_HEIGHT) {
			self->cursor_y -= FONT_HEIGHT;
			self->cursor_x = self->width - FONT_WIDTH;
		};
		// Erase the previous character by clearing its pixels
		vbe_clear_ch(self, self->cursor_x, self->cursor_y, VBE_COLOR_BLACK);
		break;
	};
	default: {
		// Standard character: Draw the character and move the cursor forward
		// Clear the area where the character will be drawn
		vbe_clear_ch(self, self->cursor_x, self->cursor_y, VBE_COLOR_BLACK);
		// Ensure the character is within the ASCII range
		if (ch < 0 || ch >= 128) {
			return;
		};
		// Retrieve the bitmap for the character
		const uint8_t* bitmap = ascii_bitmap[(uint8_t)ch];
		// Iterate over each row of the character bitmap
		for (size_t row = 0; row < FONT_HEIGHT; row++) {
			const uint8_t byte = bitmap[row];
			// Iterate over each column of the character bitmap
			for (size_t col = 0; col < FONT_WIDTH; col++) {
				// Check if the pixel should be turned on
				const bool is_pixel_on = byte & (1 << col);

				if (is_pixel_on) {
					// Draw the pixel at the calculated position with the specified color
					vbe_put_pixel_at(self, self->cursor_x + col, self->cursor_y + row, color);
				};
			};
		};
		// Update the cursor position to the right after drawing the character
		self->cursor_x += FONT_WIDTH;
		// Handle line wrapping if the cursor exceeds the display width
		if (self->cursor_x + FONT_WIDTH > self->width) {
			self->cursor_x = 0;
			self->cursor_y += FONT_HEIGHT;
		};
		break;
	};
	};
	// Check if the cursor has moved beyond the display height and perform scrolling if necessary
	if (self->cursor_y + FONT_HEIGHT > self->height) {
		// Scroll the display content upwards and clear the bottom area
		vbe_scroll(self, VBE_COLOR_BLACK);
	}
	return;
};

void vbe_draw_string(VBEDisplay* self, const char* str, const VBEColor color)
{
	while (*str) {
		vbe_draw_ch(self, *str, color);
		str++;
	};
};