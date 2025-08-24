/**
 * @file renderer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "renderer.h"
#include "kres.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* EXTERNAL API */
extern fb_t g_fb;

/* PUBLIC API */
kresult_t renderer_setup(renderer_t* renderer, font_t* font, const usize screen_w, const usize screen_h, const b8 bg_transparent);
void renderer_set_fg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);
void renderer_set_bg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);
void renderer_set_cursor(renderer_t* renderer, const usize x, const usize y);
void renderer_set_color(renderer_t* renderer, const usize fg, const usize bg);
font_t* renderer_get_font(const renderer_t* renderer);
void renderer_set_font(renderer_t* renderer, font_t* font);
void renderer_scroll(renderer_t* renderer);
void renderer_draw_ch(renderer_t* renderer, const ch glyph);
void renderer_draw_text(renderer_t* renderer, const c8* text);

/* INTERNAL API */
// -

static inline void _put(renderer_t* renderer, const usize x, const usize y, const usize color)
{
	if (x >= renderer->screen_w || y >= renderer->screen_h)
		return;
	fb_put_pixel_at(x, y, color);
};

kresult_t renderer_setup(renderer_t* renderer, font_t* font, const usize screen_w, const usize screen_h, const b8 bg_transparent)
{
	if (!renderer)
		return kresult_err(-K_EINVAL, "Renderer is NULL");

	if (!font)
		return kresult_err(-K_EINVAL, "Font is NULL");

	if (screen_w == 0 || screen_h == 0)
		return kresult_err(-K_EINVAL, "Screen dimensions invalid");

	renderer->font = font;
	renderer->screen_w = screen_w;
	renderer->screen_h = screen_h;
	renderer->bg_transparent = bg_transparent;
	renderer->bg = renderer->fg = 0x0;
	return kresult_ok(NULL);
};

void renderer_set_fg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a)
{
	renderer->fg = fb_pack_rgba(info, r, g, b, a);
};

void renderer_set_bg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a)
{
	renderer->bg = fb_pack_rgba(info, r, g, b, a);
};

void renderer_set_cursor(renderer_t* renderer, const usize x, const usize y)
{
	if (!renderer)
		return;
	renderer->cursor_x = x;
	renderer->cursor_y = y;
};

void renderer_set_color(renderer_t* renderer, const usize fg, const usize bg)
{
	if (!renderer)
		return;
	renderer->fg = fg;
	renderer->bg = bg;
};

font_t* renderer_get_font(const renderer_t* renderer) { return renderer->font; };

void renderer_set_font(renderer_t* renderer, font_t* font)
{
	if (!font || !renderer)
		return;
	renderer->font = font;
};

void renderer_draw_ch(renderer_t* renderer, const ch glyph)
{
	if (renderer->cursor_x + renderer->font->width > renderer->screen_w)
		renderer_set_cursor(renderer, 0, renderer->cursor_y + renderer->font->width);

	// Check scrolling
	if (renderer->cursor_y + renderer->font->height > renderer->screen_h)
		renderer_scroll(renderer);

	for (usize y = 0; y < renderer->font->height; y++) {
		for (usize x = 0; x < renderer->font->width; x++) {
			const b8 on = font_get_pixel_at(renderer->font, glyph, (u8)x, (u8)y);

			if (on)
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->fg);
			else if (!renderer->bg_transparent)
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->bg);
		};
	};
	renderer_set_cursor(renderer, renderer->cursor_x + renderer->font->width, renderer->cursor_y);
};

void renderer_draw_text(renderer_t* renderer, const c8* text)
{
	while (*text)
		renderer_draw_ch(renderer, *text++);
};

void renderer_scroll(renderer_t* renderer)
{
	fb_scroll(renderer->bg, renderer->font->height);

	if (renderer->cursor_y >= renderer->font->height)
		renderer->cursor_y -= renderer->font->height;
	else
		renderer->cursor_y = 0;
};