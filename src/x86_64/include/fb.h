/**
 * @file fb.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "kres.h"
#include "types.h"

typedef struct fb_boot_adapter {
	void* vaddr; // Start address of the framebuffer in memory
	u32 width;
	u32 height;
	u32 pitch; // Number of bytes per scanline (may include padding)
	u8 bpp;	   // Bits per pixel (e.g., 32 for ARGB8888)
	struct {
		u8 r_shift, g_shift, b_shift, a_shift;
		u8 r_size, g_size, b_size, a_size;
	} fmt; // Describes how color data is stored in each pixel
} fb_boot_adapter_t;

typedef struct fb {
	u32* frontbuffer;	// Points to the actual framebuffer in video memory mmio-mapped (what is visible on screen)
	u32* backbuffer;	// Optional off-screen buffer for double buffering
	fb_boot_adapter_t info; // Framebuffer layout and pixel format details provided by the bootloader
} fb_t;

void fb_setup(const fb_boot_adapter_t* adapter);
kresult_t fb_clear(const u32 color);
kresult_t fb_put_pixel_at(const usize x, const usize y, const u32 color);
void fb_scroll(const u32 color, const usize rows);
u32 fb_pack_rgba(const fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);