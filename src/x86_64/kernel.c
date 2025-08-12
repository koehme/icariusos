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
	if (!asm_sse_setup()) {
		panic();
	};
	kresult_t r = _init_limine();

	if (r.code != K_OK) {
		panic();
	};
	font_t font = {};
	font_setup(&font, 8, 8, default_glyph);

	renderer_t renderer = {};
	renderer_setup(&renderer, &font, g_fb.info.width, g_fb.info.height, true);

	renderer_set_fg_rgba(&renderer, &g_fb.info, 0x00, 0xAC, 0x00, 0xFF);
	renderer_set_bg_rgba(&renderer, &g_fb.info, 0x00, 0x00, 0x00, 0x00);

	tty_t tty = {};
	tty_init(&tty, &renderer);

	for (size_t i = 0; i < 800000000; i++)
		tty_puts(&tty, "Hello from icariusOS x64\n");

	for (;;)
		asm_hlt();
	return;
};