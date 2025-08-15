/**
 * @file renderer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "renderer.h"

/* EXTERNAL API */
extern fb_t g_fb;

/* PUBLIC API */
void renderer_setup(renderer_t* renderer, font_t* font, const uint32_t screen_w, const uint32_t screen_h, const bool bg_transparent);
void renderer_set_fg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
void renderer_set_bg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
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
	if (x >= renderer->screen_w || y >= renderer->screen_h)
		return;
	fb_put_pixel_at(x, y, color);
	return;
};

void renderer_setup(renderer_t* renderer, font_t* font, const uint32_t screen_w, const uint32_t screen_h, const bool bg_transparent)
{
	renderer->font = font;
	renderer->screen_w = screen_w;
	renderer->screen_h = screen_h;
	renderer->bg_transparent = bg_transparent;
	renderer->bg = renderer->fg = 0x0;
	return;
};

void renderer_set_fg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	renderer->fg = fb_pack_rgba(info, r, g, b, a);
	return;
};

void renderer_set_bg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	renderer->bg = fb_pack_rgba(info, r, g, b, a);
	return;
};

void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y)
{
	if (!renderer) {
		return;
	};
	renderer->cursor_x = x;
	renderer->cursor_y = y;
	return;
};

void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg)
{
	if (!renderer) {
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
		return;
	};
	renderer->font = font;
	return;
};

void renderer_draw_ch(renderer_t* renderer, const char ch)
{
	unsigned char u = (unsigned char)ch;

	if (u < 32 || u >= 128) {
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
	fb_scroll(renderer->bg, renderer->font->height);

	if (renderer->cursor_y >= renderer->font->height) {
		renderer->cursor_y -= renderer->font->height;
	} else {
		renderer->cursor_y = 0;
	};
	return;
};