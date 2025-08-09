#include "renderer.h"
#include "errno.h"
#include "fb.h"
#include "font.h"

extern int errno;
extern font_t default_font;

static inline void _put(renderer_t* renderer, const uint32_t x, const uint32_t y, const uint32_t color)
{
	if (x >= renderer->screen_w || y >= renderer->screen_h) {
		return;
	};
	fb_put_pixel_at(x, y, color);
	return;
};

void renderer_setup(renderer_t* renderer, const uint32_t screen_w, const uint32_t screen_h, const uint32_t fg, const uint32_t bg, const bool bg_transparent)
{
	renderer->screen_w = screen_w;
	renderer->screen_h = screen_h;
	renderer->fg = fg;
	renderer->bg = bg;
	renderer->bg_transparent = bg_transparent;
	return;
};

void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y)
{
	renderer->cursor_x = x;
	renderer->cursor_y = y;
	return;
};

void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg)
{
	renderer->fg = fg;
	renderer->bg = bg;
	return;
};

void renderer_draw_ch(renderer_t* renderer, const char ch)
{
	unsigned char u = (unsigned char)ch;

	if (u < 32 || u >= 128) {
		errno = EINVAL;
		return;
	};

	if (renderer->cursor_x + FONT_WIDTH > renderer->screen_w) {
		renderer_set_cursor(renderer, 0, renderer->cursor_y + FONT_WIDTH);
	};

	// Check scrolling
	if (renderer->cursor_y + default_font.height > renderer->screen_h) {
		renderer_scroll(renderer);
	};

	for (uint32_t y = 0; y < default_font.height; y++) {
		for (uint32_t x = 0; x < default_font.width; x++) {
			const bool on = font_get_pixel_at(&default_font, ch, (uint8_t)x, (uint8_t)y);

			if (on) {
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->fg);
			} else if (!renderer->bg_transparent) {
				_put(renderer, renderer->cursor_x + x, renderer->cursor_y + y, renderer->bg);
			};
		};
	};
	renderer_set_cursor(renderer, renderer->cursor_x + FONT_WIDTH, renderer->cursor_y);
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

	if (renderer->cursor_y >= FONT_HEIGHT)
		renderer->cursor_y -= FONT_HEIGHT;
	else
		renderer->cursor_y = 0;
	return;
};