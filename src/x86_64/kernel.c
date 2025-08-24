/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

#include <stdnoreturn.h>

/* EXTERNAL API */
extern fb_t g_fb;
extern const ch default_glyph[128][8];

/* PUBLIC API */
noreturn void kmain(void);

/* INTERNAL API */
static kresult_t _kinit_console(font_t* font, renderer_t* renderer, tty_t* tty);

static kresult_t _kinit_console(font_t* font, renderer_t* renderer, tty_t* tty)
{
	kresult_t res = kresult_ok(NULL);
	res = font_setup(font, 8, 8, default_glyph);

	if (res.code != K_OK)
		return res;

	res = renderer_setup(renderer, font, g_fb.info.width, g_fb.info.height, true);

	if (res.code != K_OK)
		return res;

	renderer_set_fg_rgba(renderer, &g_fb.info, 0x00, 0xAC, 0x00, 0xFF);
	renderer_set_bg_rgba(renderer, &g_fb.info, 0x00, 0x00, 0x00, 0x00);

	res = tty_init(tty, renderer);

	if (res.code != K_OK)
		return res;

	tty_set_active(tty);
	return kresult_ok(NULL);
};

noreturn void kmain(void)
{
	bootstrap_setup();

	font_t font = {};
	renderer_t renderer = {};
	tty_t tty = {};

	if (_kinit_console(&font, &renderer, &tty).code != K_OK)
		panic();

	pfa_dump(false);

	for (;;)
		asm_hlt();
};