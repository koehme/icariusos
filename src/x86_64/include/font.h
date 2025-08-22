/**
 * @file font.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "types.h"

typedef struct font {
	u32 width;
	u32 height;
	const u8 (*glyphs)[8];
} font_t;

extern const u8 default_glyph[128][8];

void font_setup(font_t* font, const u32 width, const u32 height, const u8 (*glyphs)[8]);
const u8* font_get_glyph(const font_t* font, const char ch);
bool font_get_pixel_at(const font_t* font, const char ch, const u8 pixel_x, const u8 pixel_y);