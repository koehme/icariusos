/**
 * @file fb.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fb.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
void fb_setup(const fb_boot_adapter_t* info);
kresult_t fb_clear(const uint32_t color);
kresult_t fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color);
void fb_scroll(const uint32_t color, const uint32_t rows);
uint32_t fb_pack_rgba(const fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
fb_t g_fb;

/* INTERNAL API */
// -

void fb_setup(const fb_boot_adapter_t* info)
{
	g_fb.frontbuffer = (uint32_t*)info->vaddr;
	g_fb.backbuffer = NULL;
	memcpy(&g_fb.info, info, sizeof(g_fb.info));
	return;
};

kresult_t fb_clear(const uint32_t color)
{
	if (!g_fb.frontbuffer)
		return kresult_err(-K_ENODEV, "NO Frontbuffer");

	if (g_fb.info.bpp != 32)
		return kresult_err(-K_EINVAL, "Only 32 Bpp is supported");

	for (uint32_t y = 0; y < g_fb.info.height; y++) {
		uint32_t* row = (uint32_t*)(((uint8_t*)g_fb.frontbuffer) + y * g_fb.info.pitch);

		for (uint32_t x = 0; x < g_fb.info.width; x++)
			row[x] = color;
	};
	return kresult_ok(NULL);
};

kresult_t fb_put_pixel_at(const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (!g_fb.frontbuffer)
		return kresult_err(-K_ENODEV, "NO Frontbuffer");

	if (g_fb.info.bpp != 32)
		return kresult_err(-K_EINVAL, "Only 32 Bpp is supported");

	if (x >= g_fb.info.width || y >= g_fb.info.height)
		return kresult_err(-K_EINVAL, "Display limits exceeded");

	uint32_t* row = (uint32_t*)(((uint8_t*)g_fb.frontbuffer) + y * g_fb.info.pitch);
	row[x] = color;
	return kresult_ok(NULL);
};

void fb_scroll(const uint32_t color, const uint32_t rows)
{
	const uint32_t scroll_bytes = rows * g_fb.info.pitch;
	uint8_t* src = (uint8_t*)g_fb.frontbuffer + scroll_bytes;
	uint8_t* dest = (uint8_t*)g_fb.frontbuffer;
	const uint32_t total_bytes = (g_fb.info.height - rows) * g_fb.info.pitch;

	memmove(dest, src, total_bytes);

	for (uint32_t y = g_fb.info.height - rows; y < g_fb.info.height; y++)
		for (uint32_t x = 0; x < g_fb.info.width; x++)
			fb_put_pixel_at(x, y, color);
	return;
};

uint32_t fb_pack_rgba(const fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	const uint32_t native_r = (((uint32_t)r) << info->fmt.r_shift);
	const uint32_t native_g = (((uint32_t)g) << info->fmt.g_shift);
	const uint32_t native_b = (((uint32_t)b) << info->fmt.b_shift);
	const uint32_t native_a = info->fmt.a_size ? (((uint32_t)a) << info->fmt.a_shift) : 0x0u;
	const uint32_t rgba = (native_r | native_g | native_b | native_a);
	return rgba;
};