#include "icarius.h"
#include <stdbool.h>
#include <stdint.h>

#pragma once

enum {
	BLACK = 0x00000000,
	RED = 0x00FF0000,
	GREEN = 0x0000FF00,
	BLUE = 0x000000FF,
	WHITE = 0xFFFFFFFF,
	YELLOW = 0xFFFFFF00,
};

typedef struct font {
	uint32_t width;
	uint32_t height;
	const uint8_t (*glyphs)[FONT_HEIGHT];
} font_t;

extern const uint8_t default_glyph[ASCII][FONT_HEIGHT];

const uint8_t* font_get_glyph(const font_t* font, const char ch);
bool font_get_pixel_at(const font_t* font, const char ch, const uint8_t pixel_x, const uint8_t pixel_y);