/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#include "errno.h"
#include "fb.h"
#include "font.h"
#include "kernel.h"
#include "panic.h"
#include "renderer.h"
#include "tty.h"

/* EXTERNAL API */
extern int errno;
extern fb_t g_fb;

/* PUBLIC API */
void kmain(void);
void halt(void);

/* INTERNAL API */
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};
static void _init_limine(void);

void asm_halt(void)
{
	__asm__ volatile("hlt");
	return;
};

static void _init_limine(void)
{
	errno = 0;

	if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1) {
		errno = ENODEV;
		panic();
		return;
	};
	struct limine_framebuffer* fb = fb_request.response->framebuffers[0];

	if (fb->bpp != 32) {
		errno = EINVAL;
		panic();
		return;
	};
	const fb_info_t info = {
	    .vaddr = fb->address,
	    .width = fb->width,
	    .height = fb->height,
	    .pitch = fb->pitch,
	    .bpp = fb->bpp,
	    .fmt =
		{
		    .r_shift = fb->red_mask_shift,
		    .g_shift = fb->green_mask_shift,
		    .b_shift = fb->blue_mask_shift,
		    .a_shift = 0,
		    .r_size = fb->red_mask_size,
		    .g_size = fb->green_mask_size,
		    .b_size = fb->blue_mask_size,
		    .a_size = 0,
		},
	};
	fb_setup(&info);
	return;
};

void kmain(void)
{
	_init_limine();

	font_t font;
	font_setup(&font, 8, 8, default_glyph);

	renderer_t renderer;
	renderer_setup(&renderer, &font, g_fb.info.width, g_fb.info.height, true);

	renderer_set_fg_rgba(&renderer, &g_fb.info, 0x00, 0xAC, 0x00, 0xFF);
	renderer_set_bg_rgba(&renderer, &g_fb.info, 0x00, 0x00, 0x00, 0x00);

	tty_t tty;
	tty_init(&tty, &renderer);

	for (size_t i = 0; i < 800000000; i++)
		tty_puts(&tty, "Hello from icariusOS x64\n");

	for (;;) {
		asm_halt();
	};
	return;
};