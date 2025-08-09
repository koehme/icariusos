#include "tty.h"
#include "font.h"
#include "renderer.h"
#include "string.h"
#include <stddef.h>

void tty_init(tty_t* tty, renderer_t* renderer)
{
	tty->renderer = renderer;
	return;
};

void tty_carriage_return(tty_t* tty)
{
	renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y);
	return;
};

void tty_newline(tty_t* tty)
{
	renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y + FONT_HEIGHT);

	if (tty->renderer->cursor_y + FONT_HEIGHT > tty->renderer->screen_h) {
		renderer_scroll(tty->renderer);
	};
	return;
};

void tty_backspace(tty_t* tty)
{
	// Backspace: Move cursor back and erase the previous character
	if (tty->renderer->cursor_x >= FONT_WIDTH) {
		renderer_set_cursor(tty->renderer, tty->renderer->cursor_x - FONT_WIDTH, tty->renderer->cursor_y);
	} else if (tty->renderer->cursor_y >= FONT_HEIGHT) {
		renderer_set_cursor(tty->renderer, tty->renderer->screen_w - FONT_WIDTH, tty->renderer->cursor_y - FONT_HEIGHT);
	};
	// Erase the previous character by clearing its pixels
	renderer_draw_ch(tty->renderer, BLACK);
	return;
};

void tty_putc(tty_t* tty, const char ch)
{
	switch (ch) {
	case '\n': {
		tty_newline(tty);
		break;
	};
	case '\r': {
		tty_carriage_return(tty);
		break;
	};
	case '\b': {
		tty_backspace(tty);
		break;
	};
	default: {
		renderer_draw_ch(tty->renderer, ch);
		break;
	};
	};
	return;
};

void tty_puts(tty_t* tty, const char* text)
{
	const size_t len = strlen(text);

	for (size_t i = 0; i < len; i++) {
		const char ch = text[i];
		tty_putc(tty, ch);
	};
	for (size_t j = 0; j < 5000050; j++) {
	};
	return;
};