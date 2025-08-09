/**
 * @file renderer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t screen_w, screen_h;
	uint32_t cursor_x, cursor_y;
	uint32_t fg, bg;
	bool bg_transparent;
} renderer_t;

void renderer_setup(renderer_t* renderer, const uint32_t screen_w, const uint32_t screen_h, const uint32_t fg, const uint32_t bg, const bool bg_transparent);
void renderer_set_cursor(renderer_t* renderer, const uint32_t x, const uint32_t y);
void renderer_set_color(renderer_t* renderer, const uint32_t fg, const uint32_t bg);
void renderer_scroll(renderer_t* renderer);
void renderer_draw_ch(renderer_t* r, const char ch);
void renderer_draw_text(renderer_t* renderer, const char* text);