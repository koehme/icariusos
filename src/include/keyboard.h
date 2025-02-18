/**
 * @file kbd.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "icarius.h"
#include "kernel.h"

typedef struct kbd {
	bool caps;
	bool caps_lock;
	bool alt_gr;
} kbd_t;

void kbd_init();
void kbd_handler(void* dev, const uint8_t data);
uint8_t kbd_translate(const uint8_t scancode);

#endif