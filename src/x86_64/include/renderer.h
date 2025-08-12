/**
 * @file renderer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "fb.h"
#include "font.h"
#include "kerr.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	font_t* font;
	uint32_t screen_w, screen_h;
	uint32_t cursor_x, cursor_y;
	uint32_t fg, bg;
	bool bg_transparent;
} renderer_t;

void renderer_setup(renderer_t* renderer, font_t* font, const uint32_t screen_w, const uint32_t screen_h, const bool bg_transparent);
void renderer_set_fg_rgba(renderer_t* renderer, fb_info_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
void renderer_set_bg_rgba(renderer_t* renderer, fb_info_t* info, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y);
void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg);
font_t* renderer_get_font(const renderer_t* renderer);
void renderer_set_font(renderer_t* renderer, font_t* font);
void renderer_scroll(renderer_t* renderer);
void renderer_draw_ch(renderer_t* r, const char ch);
void renderer_draw_text(renderer_t* renderer, const char* text);