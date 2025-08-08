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
#include "kernel.h"
#include "panic.h"

/* EXTERNAL API */
extern int errno;

/* PUBLIC API */
void kmain(void);
void halt(void);

/* INTERNAL API */
static void _init_limine(void);
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void halt(void) { __asm__ volatile("hlt"); };

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
	const fb_color_format_t format = {
	    .r_shift = fb->red_mask_shift,
	    .g_shift = fb->green_mask_shift,
	    .b_shift = fb->blue_mask_shift,
	    .a_shift = 0,
	    .r_size = fb->red_mask_size,
	    .g_size = fb->green_mask_size,
	    .b_size = fb->blue_mask_size,
	    .a_size = 0,
	};
	fb_setup(fb->address, fb->width, fb->height, fb->pitch, fb->bpp, format);
	fb_clear(GREEN);
	return;
};

void kmain(void)
{
	_init_limine();

	for (;;)
		halt();
	return;
};