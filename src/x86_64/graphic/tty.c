/**
 * @file tty.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "tty.h"
#include "font.h"
#include "string.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t tty_init(tty_t* tty, renderer_t* renderer);
void tty_set_tabwidth(tty_t* tty, const usize tab_width);
void tty_putc(tty_t* tty, const ch ch);
void tty_puts(tty_t* tty, const ch* text);
void tty_set_active(tty_t* tty);
tty_t* tty_get_active(void);

/* INTERNAL API */
static void _handle_carriage_return(tty_t* tty);
static void _handle_tab(tty_t* tty);
static void _handle_newline(tty_t* tty);
static void _handle_backspace(tty_t* tty);
static tty_t* active_tty = NULL;

kresult_t tty_init(tty_t* tty, renderer_t* renderer)
{
	if (!tty)
		return kresult_err(-K_EINVAL, "Tty is NULL");

	if (!renderer)
		return kresult_err(-K_EINVAL, "Renderer is NULL");

	tty->renderer = renderer;
	tty->tab_width = 8;
	return kresult_ok(NULL);
}

void tty_set_tabwidth(tty_t* tty, const usize tab_width)
{
	if (tab_width <= 0)
		tty->tab_width = 1;
	tty->tab_width = tab_width;
};

static void _handle_carriage_return(tty_t* tty) { renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y); };

static void _handle_tab(tty_t* tty)
{
	const usize curr_col = tty->renderer->cursor_x / tty->renderer->font->width;
	const usize next_col = ((curr_col / tty->tab_width) + 1) * tty->tab_width;
	const usize next_pixel_x = next_col * tty->renderer->font->width;

	if (next_pixel_x >= tty->renderer->screen_w)
		renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y + tty->renderer->font->height);
	renderer_set_cursor(tty->renderer, next_pixel_x, tty->renderer->cursor_y);
};

static void _handle_newline(tty_t* tty)
{
	renderer_set_cursor(tty->renderer, 0, tty->renderer->cursor_y + tty->renderer->font->height);

	if (tty->renderer->cursor_y + tty->renderer->font->height > tty->renderer->screen_h)
		renderer_scroll(tty->renderer);
};

static void _handle_backspace(tty_t* tty)
{
	// Backspace: Move cursor back and erase the previous character
	if (tty->renderer->cursor_x >= tty->renderer->font->width)
		renderer_set_cursor(tty->renderer, tty->renderer->cursor_x - tty->renderer->font->width, tty->renderer->cursor_y);
	else if (tty->renderer->cursor_y >= tty->renderer->font->height)
		renderer_set_cursor(tty->renderer, tty->renderer->screen_w - tty->renderer->font->width, tty->renderer->cursor_y - tty->renderer->font->height);
	// Erase the previous character by clearing its pixels
	renderer_draw_ch(tty->renderer, ' ');
};

void tty_putc(tty_t* tty, const ch ch)
{
	switch (ch) {
	case '\t':
		_handle_tab(tty);
		break;
	case '\n':
		_handle_newline(tty);
		break;
	case '\r':
		_handle_carriage_return(tty);
		break;
	case '\b':
		_handle_backspace(tty);
		break;
	default:
		renderer_draw_ch(tty->renderer, ch);
		break;
	};
};

void tty_puts(tty_t* tty, const ch* text)
{
	const usize len = strlen(text);

	for (usize i = 0; i < len; i++) {
		const c8 ch = text[i];
		tty_putc(tty, ch);
	};
};

void tty_set_active(tty_t* tty) { active_tty = tty; };

tty_t* tty_get_active(void) { return active_tty; }