/**
 * @file tty.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "renderer.h"
#include "types.h"

#include <stddef.h>

typedef struct {
	renderer_t* renderer;
	usize tab_width;
} tty_t;

void tty_init(tty_t* tty, renderer_t* renderer);
void tty_putc(tty_t* tty, const ch ch);
void tty_puts(tty_t* tty, const ch* text);
void tty_set_tabwidth(tty_t* tty, const usize tab_width);
void tty_set_active(tty_t* tty);
tty_t* tty_get_active(void);