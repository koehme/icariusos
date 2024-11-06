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

void vbe_put_pixel_at(VBEDisplay* self, const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (x >= self->width || y >= self->height) {
		return;
	};
	const uint32_t bytes_per_pixel = self->bpp / 8;
	const uint32_t pixel_offset = y * self->pitch + x * bytes_per_pixel;
	const uint8_t* pixel_address = (uint8_t*)self->addr + pixel_offset;
	*((uint32_t*)pixel_address) = color;
	return;
};

void vbe_draw_horizontal_line(VBEDisplay* self, const uint32_t y, const uint32_t color)
{
	if (y >= self->height) {
		return;
	};

	for (uint32_t x = 0; x < self->width; ++x) {
		vbe_put_pixel_at(self, x, y, color);
	};
	return;
};