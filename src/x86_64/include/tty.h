/**
 * @file tty.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "renderer.h"
#include <stddef.h>

typedef struct {
	renderer_t* renderer;
	size_t tab_width;
} tty_t;

void tty_init(tty_t* tty, renderer_t* renderer);
void tty_putc(tty_t* tty, const char ch);
void tty_puts(tty_t* tty, const char* text);
void tty_set_tabwidth(tty_t* tty, const size_t tab_width);