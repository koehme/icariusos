#include "kbd.h"
#include "stdio.h"

static bool shift = false;

static const char scancode_ascii[128] = {0,    27,  '1', '2',  '3', '4', '5', '6', '7',	 '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
					 't',  'z', 'u', 'i',  'o', 'p', '[', ']', '\n', 0,   'a', 's', 'd', 'f', 'g',	'h',  'j', 'k', 'l', ':',
					 '\'', '`', 0,	 '\\', 'y', 'x', 'c', 'v', 'b',	 'n', 'm', ',', '.', '/', 0,	'*',  0,   ' '};

static const char scancode_ascii_shift[128] = {0,   27,	 '!', '"', '#', '$', '%', '&', '/',  '(', ')', '=', '?', '`', '\b', '\t', 'Q', 'W', 'E', 'R',
					       'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,	  'A', 'S', 'D', 'F', 'G',  'H',  'J', 'K', 'L', ':',
					       '"', '~', 0,   '|', 'Y', 'X', 'C', 'V', 'B',  'N', 'M', '<', ':', '_', 0,    '*',  0,   ' '};

key_event_t translate_sc_into_event(const uint8_t scancode)
{
	key_event_t event = {};

	const bool is_keyup = scancode & 0b10000000;
	const uint8_t code = scancode & 0b01111111;

	if (code == SC_LSHIFT || code == SC_RSHIFT) {
		shift = !is_keyup;
		event.pressed = false;
		return event;
	};
	event.pressed = !is_keyup;

	switch (code) {
	case SC_ENTER:
		event.code = KEY_ENTER;
		break;
	case SC_BACKSPACE:
		event.code = KEY_BACKSPACE;
		break;
	case SC_ESC:
		event.code = KEY_ESC;
		break;
	case SC_ARROW_UP:
		event.code = KEY_UP;
		break;
	case SC_ARROW_DOWN:
		event.code = KEY_DOWN;
		break;
	case SC_ARROW_LEFT:
		event.code = KEY_LEFT;
		break;
	case SC_ARROW_RIGHT:
		event.code = KEY_RIGHT;
		break;
	default:
		event.code = KEY_OTHER;
		break;
	};
	if (event.pressed && code < 128) {
		const char ch = shift ? scancode_ascii_shift[code] : scancode_ascii[code];

		if (ch) {
			event.ascii = ch;
		};
	};
	return event;
};