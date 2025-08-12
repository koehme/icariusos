/**
 * @file font.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "icarius.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct font {
	uint32_t width;
	uint32_t height;
	const uint8_t (*glyphs)[8];
} font_t;

extern const uint8_t default_glyph[128][8];

void font_setup(font_t* font, const uint32_t width, const uint32_t height, const uint8_t (*glyphs)[8]);
const uint8_t* font_get_glyph(const font_t* font, const char ch);
bool font_get_pixel_at(const font_t* font, const char ch, const uint8_t pixel_x, const uint8_t pixel_y);