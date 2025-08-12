/**
 * @file tty.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "tty.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
void tty_init(tty_t* tty, renderer_t* renderer);
void tty_set_tabwidth(tty_t* tty, const size_t tab_width);
void tty_putc(tty_t* tty, const char ch);
void tty_puts(tty_t* tty, const char* text);

/* INTERNAL API */
static void _carriage_return(tty_t* tty);
static void _tab(tty_t* tty);
static void _newline(tty_t* tty);
static void _backspace(tty_t* tty);

void tty_init(tty_t* tty, renderer_t* renderer)
{
	tty->renderer = renderer;
	tty->tab_width = 8;
	return;
};

void tty_set_tabwidth(tty_t* tty, const size_t tab_width)
{
	if (tab_width <= 0) {
		tty->tab_width = 1;
		return;
	};
	tty->tab_width = tab_width;
	return;
};

static void _carriage_return(tty_t* tty)
{
	renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y);
	return;
};

static void _tab(tty_t* tty)
{
	const size_t curr_col = tty->renderer->cursor_x / tty->renderer->font->width;
	const size_t next_col = ((curr_col / tty->tab_width) + 1) * tty->tab_width;
	const size_t next_pixel_x = next_col * tty->renderer->font->width;

	if (next_pixel_x >= tty->renderer->screen_w) {
		renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y + tty->renderer->font->height);
		return;
	};
	renderer_set_cursor(tty->renderer, next_pixel_x, tty->renderer->cursor_y);
	return;
};

static void _newline(tty_t* tty)
{
	renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y + tty->renderer->font->height);

	if (tty->renderer->cursor_y + tty->renderer->font->height > tty->renderer->screen_h)
		renderer_scroll(tty->renderer);
	return;
};

static void _backspace(tty_t* tty)
{
	// Backspace: Move cursor back and erase the previous character
	if (tty->renderer->cursor_x >= tty->renderer->font->width) {
		renderer_set_cursor(tty->renderer, tty->renderer->cursor_x - tty->renderer->font->width, tty->renderer->cursor_y);
	} else if (tty->renderer->cursor_y >= tty->renderer->font->height) {
		renderer_set_cursor(tty->renderer, tty->renderer->screen_w - tty->renderer->font->width, tty->renderer->cursor_y - tty->renderer->font->height);
	};
	// Erase the previous character by clearing its pixels
	renderer_draw_ch(tty->renderer, ' ');
	return;
};

void tty_putc(tty_t* tty, const char ch)
{
	switch (ch) {
	case '\t': {
		_tab(tty);
		break;
	};
	case '\n': {
		_newline(tty);
		break;
	};
	case '\r': {
		_carriage_return(tty);
		break;
	};
	case '\b': {
		_backspace(tty);
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

		// Testing :)
		for (size_t i = 0; i < 50000; i++)
			;
	};
	return;
};