/**
 * @file renderer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "renderer.h"
#include "errno.h"
#include "fb.h"

/* EXTERNAL API */
extern int errno;

/* PUBLIC API */
void renderer_setup(renderer_t* renderer, font_t* font, const uint32_t screen_w, const uint32_t screen_h, const uint32_t fg, const uint32_t bg,
		    const bool bg_transparent);
void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y);
void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg);
font_t* renderer_get_font(const renderer_t* renderer);
void renderer_set_font(renderer_t* renderer, font_t* font);
void renderer_scroll(renderer_t* renderer);
void renderer_draw_ch(renderer_t* r, const char ch);
void renderer_draw_text(renderer_t* renderer, const char* text);

/* INTERNAL API */
// -

static inline void _put(renderer_t* renderer, const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (x >= renderer->screen_w || y >= renderer->screen_h) {
		return;
	};
	fb_put_pixel_at(x, y, color);
	return;
};

void renderer_setup(renderer_t* renderer, font_t* font, const uint32_t screen_w, const uint32_t screen_h, const uint32_t fg, const uint32_t bg,
		    const bool bg_transparent)
{
	renderer->font = font;
	renderer->screen_w = screen_w;
	renderer->screen_h = screen_h;
	renderer->fg = fg;
	renderer->bg = bg;
	renderer->bg_transparent = bg_transparent;
	return;
};

void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y)
{
	if (!renderer) {
		errno = EINVAL;
		return;
	};
	renderer->cursor_x = x;
	renderer->cursor_y = y;
	return;
};

void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg)
{
	if (!renderer) {
		errno = EINVAL;
		return;
	};
	renderer->fg = fg;
	renderer->bg = bg;
	return;
};

font_t* renderer_get_font(const renderer_t* renderer) { return renderer->font; };

void renderer_set_font(renderer_t* renderer, font_t* font)
{
	if (!font || !renderer) {
		errno = EINVAL;
		return;
	};
	renderer->font = font;
	return;
};

void renderer_draw_ch(renderer_t* renderer, const char ch)
{
	unsigned char u = (unsigned char)ch;

	if (u < 32 || u >= 128) {
		errno = EINVAL;
		return;
	};

	if (renderer->cursor_x + renderer->font->width > renderer->screen_w) {
		renderer_set_cursor(renderer, 0, renderer->cursor_y + renderer->font->width);
	};

	// Check scrolling
	if (renderer->cursor_y + renderer->font->height > renderer->screen_h) {
		renderer_scroll(renderer);
	};

	for (size_t y = 0; y < renderer->font->height; y++) {
		for (size_t x = 0; x < renderer->font->width; x++) {
			const bool on = font_get_pixel_at(renderer->font, ch, (uint8_t)x, (uint8_t)y);

			if (on) {
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->fg);
			} else if (!renderer->bg_transparent) {
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->bg);
			};
		};
	};
	renderer_set_cursor(renderer, renderer->cursor_x + renderer->font->width, renderer->cursor_y);
	return;
};

void renderer_draw_text(renderer_t* renderer, const char* text)
{
	while (*text) {
		unsigned char u = (unsigned char)*text++;

		if (u >= 32 && u < 128) {
			renderer_draw_ch(renderer, (char)u);
		};
	};
	return;
};

void renderer_scroll(renderer_t* renderer)
{
	fb_scroll(renderer->bg);

	if (renderer->cursor_y >= renderer->font->height) {
		renderer->cursor_y -= renderer->font->height;
	} else {
		renderer->cursor_y = 0;
	};
	return;
};