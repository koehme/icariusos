/**
 * @file fb.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fb.h"
#include "icarius.h"
#include "string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* EXTERNAL API */
// -

/* PUBLIC API */
void fb_setup(const fb_boot_adapter_t* adapter);
kresult_t fb_clear(const u32 color);
kresult_t fb_put_pixel_at(const usize x, const usize y, const u32 color);
void fb_scroll(const u32 color, const usize rows);
u32 fb_pack_rgba(const fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);
fb_t g_fb;

/* INTERNAL API */
// -

void fb_setup(const fb_boot_adapter_t* adapter)
{
	g_fb.frontbuffer = (u32*)adapter->vaddr;
	g_fb.backbuffer = NULL;
	memcpy(&g_fb.info, adapter, sizeof(g_fb.info));
};

kresult_t fb_clear(const u32 color)
{
	if (!g_fb.frontbuffer)
		return kresult_err(-K_ENODEV, "No frontbuffer available");

	if (g_fb.info.bpp != 32)
		return kresult_err(-K_EINVAL, "Only 32 bpp are supported");

	for (u32 y = 0; y < g_fb.info.height; y++) {
		u32* row = (u32*)(((u8*)g_fb.frontbuffer) + y * g_fb.info.pitch);

		for (u32 x = 0; x < g_fb.info.width; x++)
			row[x] = color;
	};
	return kresult_ok(NULL);
};

kresult_t fb_put_pixel_at(const usize x, const usize y, const u32 color)
{
	if (!g_fb.frontbuffer)
		return kresult_err(-K_ENODEV, "No frontbuffer available");

	if (g_fb.info.bpp != 32)
		return kresult_err(-K_EINVAL, "Only 32 bpp are supported");

	if (x >= g_fb.info.width || y >= g_fb.info.height)
		return kresult_err(-K_EINVAL, "Display limits exceeded");

	u32* row = (u32*)(((u8*)g_fb.frontbuffer) + y * g_fb.info.pitch);
	row[x] = color;
	return kresult_ok(NULL);
};

void fb_scroll(const u32 color, const usize rows)
{
	if (rows == 0 || rows > g_fb.info.height)
		return;

	const usize scroll_bytes = rows * g_fb.info.pitch;
	u8* src = (u8*)g_fb.frontbuffer + scroll_bytes;
	u8* dest = (u8*)g_fb.frontbuffer;
	const usize total_bytes = (g_fb.info.height - rows) * g_fb.info.pitch;

	if (total_bytes > UINT32_MAX)
		return;

	memmove(dest, src, total_bytes);

	for (usize y = g_fb.info.height - rows; y < g_fb.info.height; y++)
		for (usize x = 0; x < g_fb.info.width; x++)
			fb_put_pixel_at(x, y, color);
};

u32 fb_pack_rgba(const fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a)
{
	const u32 native_r = (((u32)r) << info->fmt.r_shift);
	const u32 native_g = (((u32)g) << info->fmt.g_shift);
	const u32 native_b = (((u32)b) << info->fmt.b_shift);
	const u32 native_a = info->fmt.a_size ? (((u32)a) << info->fmt.a_shift) : 0x0u;
	const u32 rgba = (native_r | native_g | native_b | native_a);
	return rgba;
};