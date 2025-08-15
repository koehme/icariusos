/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

/* EXTERNAL API */
extern fb_t g_fb;

/* PUBLIC API */
void kmain(void);

void kmain(void)
{
	bootstrap_setup();

	font_t font = {};
	font_setup(&font, 8, 8, default_glyph);

	renderer_t renderer = {};
	renderer_setup(&renderer, &font, g_fb.info.width, g_fb.info.height, true);

	renderer_set_fg_rgba(&renderer, &g_fb.info, 0x00, 0xAC, 0x00, 0xFF);
	renderer_set_bg_rgba(&renderer, &g_fb.info, 0x00, 0x00, 0x00, 0x00);

	tty_t tty = {};
	tty_init(&tty, &renderer);
	tty_set_active(&tty);

	pfa_dump();

	for (;;)
		asm_hlt();
	return;
};