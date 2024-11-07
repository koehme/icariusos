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

void vbe_draw_ch(VBEDisplay* self, char ch, const VBEColor color)
{
	switch (ch) {
	case '\n':
		// Neue Zeile: Cursor an den Anfang der nächsten Zeile setzen
		self->cursor_x = 0;
		self->cursor_y += FONT_HEIGHT;
		break;
	case '\r':
		// Wagenrücklauf: Cursor an den Anfang der aktuellen Zeile setzen
		self->cursor_x = 0;
		break;
	case '\b':
		// Rückschritt: Cursor zurückbewegen und vorheriges Zeichen löschen
		if (self->cursor_x >= FONT_WIDTH) {
			self->cursor_x -= FONT_WIDTH;
		} else if (self->cursor_y >= FONT_HEIGHT) {
			self->cursor_y -= FONT_HEIGHT;
			self->cursor_x = self->width - FONT_WIDTH;
		}
		// Lösche das vorherige Zeichen
		vbe_clear_ch(self, self->cursor_x, self->cursor_y, VBE_COLOR_BLACK);
		break;
	default:
		// Standardzeichen: Zeichnen und Cursor weiterbewegen
		// Zeichne das Zeichen an der aktuellen Cursor-Position
		vbe_clear_ch(self, self->cursor_x, self->cursor_y, VBE_COLOR_BLACK);

		if (ch < 0 || ch >= 128) {
			return;
		};

		const uint8_t* bitmap = ascii_bitmap[(uint8_t)ch];

		for (size_t row = 0; row < FONT_HEIGHT; row++) {
			const uint8_t byte = bitmap[row];

			for (size_t col = 0; col < FONT_WIDTH; col++) {
				const bool is_pixel_on = byte & (1 << col);

				if (is_pixel_on) {
					vbe_put_pixel_at(self, self->cursor_x + col, self->cursor_y + row, color);
				};
			};
		};

		// Cursor-Position aktualisieren
		self->cursor_x += FONT_WIDTH;

		// Zeilenumbruch, wenn das Ende der Zeile erreicht ist
		if (self->cursor_x + FONT_WIDTH > self->width) {
			self->cursor_x = 0;
			self->cursor_y += FONT_HEIGHT;
		};
		break;
	}

	if (self->cursor_y + FONT_HEIGHT > self->height) {
		// TODO SCROLL
		self->cursor_y = 0;
	};
	return;
}

void vbe_draw_string(VBEDisplay* self, const char* str, const VBEColor color)
{
	while (*str) {
		vbe_draw_ch(self, *str, color);
		str++;
	};
};