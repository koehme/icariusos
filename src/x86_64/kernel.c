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
	fb_clear(BLACK);
	return;
};

void kmain(void)
{
	_init_limine();
	fb_t* fb = fb_get();
	renderer_t renderer;
	renderer_setup(&renderer, fb->width, fb->height, WHITE, BLACK, true);

	tty_t tty;
	tty_init(&tty, &renderer);

	for (size_t i = 0; i < 45000; i++)
		tty_puts(&tty, "Hello World");

	for (;;) {
		asm_halt();
	};
	return;
};