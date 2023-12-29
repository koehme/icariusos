/**
 * @file kbd.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef enum KeyboardPorts
{
    KB_DATA = 0x60,
    KB_COMMAND = 0x64,
} KeyboardPorts;

typedef struct Keyboard
{
    bool caps;
    bool caps_lock;
    bool alt_gr;
} Keyboard;

void keyboard_read();

#endif