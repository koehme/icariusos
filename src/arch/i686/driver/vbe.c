/**
 * @file vbe.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vbe.h"

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

static void vbe_draw_ch(VBEDisplay* self, const uint32_t x, const uint32_t y, const char ch, const VBEColor color)
{
	vbe_clear_ch(self, x, y, VBE_COLOR_BLACK);
	// Check if character is within valid ASCII range
	if (ch < 0 || ch >= 128) {
		return;
	};
	// Get the bitmap data for the requested 'ch' character
	const uint8_t* bitmap = ascii_bitmap[(uint8_t)ch];
	// Loop through each row of the character (height is FONT_HEIGHT)
	for (size_t row = 0; row < FONT_HEIGHT; row++) {
		const uint8_t byte = bitmap[row];
		// Loop through each column in the row (width is FONT_WIDTH, typically 8)
		for (size_t col = 0; col < FONT_WIDTH; col++) {
			// Extract the specific bit from the byte for the current column position
			const bool is_pixel_on = byte & (1 << col);

			if (is_pixel_on) {
				// If bit is set, draw a pixel on the screen at the specified position
				vbe_put_pixel_at(self, x + col, y + row, color);
			};
		};
	};
	return;
};

void vbe_draw_string(VBEDisplay* self, const char* str, const VBEColor color)
{
	while (*str) {
		if (*str == '\n') {
			// Handle newline character: move the cursor to the beginning of the next line
			// Reset cursor_x to the beginning of the line
			self->cursor_x = 0;
			// Move the cursor down by the height of one character, effectively moving to the next line
			self->cursor_y += FONT_HEIGHT;
		} else if (*str == '\b') {
			// Handle backspace character: move the cursor back and clear the previous character
			// If cursor is not at the beginning of the current line, move it back by one character width
			if (self->cursor_x >= FONT_WIDTH) {
				self->cursor_x -= FONT_WIDTH;
			} else if (self->cursor_y >= FONT_HEIGHT) {
				// If cursor is at the start of the line, move it to the end of the previous line (if possible)
				// Move cursor up by one line
				self->cursor_y -= FONT_HEIGHT;
				// Set cursor to the last character position of the previous line
				self->cursor_x = self->width - FONT_WIDTH;
			};
			// Clear the character by drawing a space at the current cursor position
			// This effectively "erases" the character that was previously there
			vbe_draw_ch(self, self->cursor_x, self->cursor_y, ' ', color);
		} else if (*str == '\r') {
			// Handle carriage return character: move cursor to the beginning of the current line
			// Set cursor_x to 0 to move to the start of the current line
			self->cursor_x = 0;
		} else {
			// Handle standard characters
			// Draw the character at the current cursor position
			vbe_draw_ch(self, self->cursor_x, self->cursor_y, *str, color);
			// Move the cursor to the right by the width of one character
			self->cursor_x += FONT_WIDTH;
			// If the cursor moves past the width of the screen, move to the next line
			if (self->cursor_x >= self->width) {
				// Reset cursor_x to the start of the next line
				self->cursor_x = 0;
				// Move cursor down by the height of one character
				self->cursor_y += FONT_HEIGHT;
			};
		};
		// Move to the next character in the input string
		str++;
	};
	// Update the cursor position after the entire string is drawn
	cursor_set(self->cursor_y, self->cursor_x);
	return;
};