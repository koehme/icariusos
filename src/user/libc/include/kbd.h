#ifndef KBD_H
#define KBD_H

#include <stdbool.h>
#include <stdint.h>

/* SCANCODES */
#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_ENTER 0x1C
#define SC_BACKSPACE 0x0E
#define SC_ESC 0x01
#define SC_ARROW_UP 0x48
#define SC_ARROW_DOWN 0x50
#define SC_ARROW_LEFT 0x4B
#define SC_ARROW_RIGHT 0x4D
#define SC_DOT 0x34
#define SC_COLON 0x27
#define SC_SLASH 0x35

#define KEY_NONE 0
#define KEY_ENTER 1
#define KEY_BACKSPACE 2
#define KEY_TAB 3
#define KEY_ESC 4
#define KEY_UP 5
#define KEY_DOWN 6
#define KEY_LEFT 7
#define KEY_RIGHT 8
#define KEY_OTHER 9

typedef struct key_event {
	bool pressed;
	char ascii;
	uint8_t code;
} key_event_t;

key_event_t translate_sc_into_event(const uint8_t scancode);

#endif