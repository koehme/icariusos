/**
 * @file font.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "kres.h"
#include "types.h"

typedef struct font {
	u32 width;
	u32 height;
	const ch (*glyphs)[8];
} font_t;

kresult_t font_setup(font_t* font, const usize width, const usize height, const ch (*glyphs)[8]);
const ch* font_get_glyph(const font_t* font, const ch c);
b8 font_get_pixel_at(const font_t* font, const ch c, const usize pixel_x, const usize pixel_y);