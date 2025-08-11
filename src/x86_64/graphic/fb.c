/**
 * @file fb.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "errno.h"
#include "fb.h"
#include "string.h"

/* EXTERNAL API */
extern int errno;

/* PUBLIC API */
void fb_setup(const fb_info_t* info);
void fb_clear(const uint32_t color);
void fb_scroll(const uint32_t color, const uint32_t rows);
void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);
uint32_t fb_pack_rgba(const fb_info_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
fb_t g_fb;

/* INTERNAL API */
// -

void fb_setup(const fb_info_t* info)
{
	g_fb.frontbuffer = (uint32_t*)info->vaddr;
	g_fb.backbuffer = NULL;
	memcpy(&g_fb.info, info, sizeof(fb_info_t));
	return;
};

void fb_clear(const uint32_t color)
{
	if (!g_fb.frontbuffer) {
		errno = ENODEV;
		return;
	};

	for (uint32_t y = 0; y < g_fb.info.height; y++) {
		uint32_t* row = (uint32_t*)(((uint8_t*)g_fb.frontbuffer) + y * g_fb.info.pitch);

		for (uint32_t x = 0; x < g_fb.info.width; x++) {
			row[x] = color;
		};
	};
	return;
};

void fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (!g_fb.frontbuffer) {
		errno = ENODEV;
		return;
	};
	// Check if the coordinates are within the display's resolution boundaries
	if (x >= g_fb.info.width || y >= g_fb.info.height) {
		errno = EINVAL;
		return;
	};
	uint32_t* row = (uint32_t*)(((uint8_t*)g_fb.frontbuffer) + y * g_fb.info.pitch);
	row[x] = color;
	return;
};

void fb_scroll(const uint32_t color, const uint32_t rows)
{
	const uint32_t scroll_bytes = rows * g_fb.info.pitch;
	uint8_t* src = (uint8_t*)g_fb.frontbuffer + scroll_bytes;
	uint8_t* dest = (uint8_t*)g_fb.frontbuffer;
	const uint32_t total_bytes = (g_fb.info.height - rows) * g_fb.info.pitch;

	memmove(dest, src, total_bytes);

	for (uint32_t y = g_fb.info.height - rows; y < g_fb.info.height; y++) {
		for (uint32_t x = 0; x < g_fb.info.width; x++) {
			fb_put_pixel_at(x, y, color);
		};
	};
	return;
};

uint32_t fb_pack_rgba(const fb_info_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	const uint32_t native_r = (((uint32_t)r) << info->fmt.r_shift);
	const uint32_t native_g = (((uint32_t)g) << info->fmt.g_shift);
	const uint32_t native_b = (((uint32_t)b) << info->fmt.b_shift);
	const uint32_t native_a = info->fmt.a_size ? (((uint32_t)a) << info->fmt.a_shift) : 0x0u;
	const uint32_t rgba = (native_r | native_g | native_b | native_a);
	return rgba;
};