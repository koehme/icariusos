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
} kbd_t;

void kbd_init(kbd_t* self);
uint8_t kbd_translate(const uint8_t scancode);

#endif