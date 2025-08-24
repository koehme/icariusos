/**
 * @file renderer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "fb.h"
#include "font.h"
#include "types.h"

typedef struct {
	font_t* font;
	u32 screen_w, screen_h;
	u32 cursor_x, cursor_y;
	u32 fg, bg;
	b8 bg_transparent;
} renderer_t;

kresult_t renderer_setup(renderer_t* renderer, font_t* font, const u32 screen_w, const u32 screen_h, const b8 bg_transparent);
void renderer_set_fg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);
void renderer_set_bg_rgba(renderer_t* renderer, fb_boot_adapter_t* info, const u8 r, const u8 g, const u8 b, const u8 a);
void renderer_set_cursor(renderer_t* renderer, const u32 x, const u32 y);
void renderer_set_color(renderer_t* renderer, const u32 fg, const u32 bg);
font_t* renderer_get_font(const renderer_t* renderer);
void renderer_set_font(renderer_t* renderer, font_t* font);
void renderer_scroll(renderer_t* renderer);
void renderer_draw_ch(renderer_t* renderer, const ch glyph);
void renderer_draw_text(renderer_t* renderer, const c8* text);