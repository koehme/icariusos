/**
 * @file vbe.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vbe.h"

#include <stddef.h>

VBEDisplay vbe_display = {
    .addr = 0x0,
    .width = 0,
    .height = 0,
    .pitch = 0,
    .bpp = 0,
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

void vbe_put_pixel_at(VBEDisplay* self, const uint32_t x, const uint32_t y, const VBEColor color)
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

void vbe_draw_horizontal_line(VBEDisplay* self, const uint32_t y, const VBEColor color)
{
	if (y >= self->height) {
		return;
	};

	for (size_t x = 0; x < self->width; x++) {
		vbe_put_pixel_at(self, x, y, color);
	};
	return;
};

void vbe_draw_char(VBEDisplay* self, const uint32_t x, const uint32_t y, const char ch, const VBEColor color)
{
	// Check if character is within valid ASCII range
	if (ch < 0 || ch >= 128) {
		return; // Ignore invalid character
	};
	// Get the bitmap data for the ASCII character
	const uint8_t* bitmap = ascii_bitmap[(uint8_t)ch];
	// Loop through each row of the character (height is CHAR_HEIGHT)
	for (size_t row = 0; row < CHAR_HEIGHT; row++) {
		const uint8_t byte = bitmap[row];
		// Loop through each column in the row (width is CHAR_WIDTH, typically 8)
		for (size_t col = 0; col < CHAR_WIDTH; col++) {
			// Check if the current bit in the row byte is set (1 = pixel on)
			if (byte & (1 << (CHAR_WIDTH - 1 - col))) {
				// If bit is set, draw a pixel on the screen at the specified position
				vbe_put_pixel_at(self, x + col, y + row, color);
			};
		};
	};
	return;
};