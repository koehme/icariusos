/**
 * @file kbd.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "ps2.h"
#include "string.h"

/* EXTERNAL API */
extern void asm_irq1_keyboard(void);

/* PUBLIC API */
void kbd_init(kbd_t* self);
void kbd_handler(void* dev, const uint8_t data);

/* INTERNAL API */
static void _process_keystroke(kbd_t* self, const uint8_t makecode, const uint8_t breakcode);

kbd_t kbd = {
    .caps = false,
    .caps_lock = false,
    .alt_gr = false,
};

const static uint8_t qwertz_lower[] = {
    0,	  // 0
    0,	  // 1
    '1',  // 2
    '2',  // 3
    '3',  // 4
    '4',  // 5
    '5',  // 6
    '6',  // 7
    '7',  // 8
    '8',  // 9
    '9',  // 10
    '0',  // 11
    223,  // 12 - 'ß'
    180,  // 13 - '´'
    '\b', // 14 - Backspace
    9,	  // 15 - Tab
    'q',  // 16
    'w',  // 17
    'e',  // 18
    'r',  // 19
    't',  // 20
    'z',  // 21
    'u',  // 22
    'i',  // 23
    'o',  // 24
    'p',  // 25
    252,  // 26 - 'ü'
    '+',  // 27
    13,	  // 28 - Enter
    0,	  // 29 - Left Strg
    'a',  // 30
    's',  // 31
    'd',  // 32
    'f',  // 33
    'g',  // 34
    'h',  // 35
    'j',  // 36
    'k',  // 37
    'l',  // 38
    246,  // 39 - 'ö'
    228,  // 40 - 'ä'
    '^',  // 41
    0,	  // 42 - Left Shift
    '<',  // 43
    'y',  // 44
    'x',  // 45
    'c',  // 46
    'v',  // 47
    'b',  // 48
    'n',  // 49
    'm',  // 50
    ',',  // 51
    '.',  // 52
    '-',  // 53
    0,	  // 54 - Right Shift
    0,	  // 55 - '*'-Key (Number)
    0,	  // 56 - Alt
    ' ',  // 57 - Space
    0	  // 58 - Caps Lock
};

const static uint8_t qwertz_upper[] = {
    0,	  // 0
    0,	  // 1
    '!',  // 2 - Shift + '1'
    '"',  // 3 - Shift + '2'
    167,  // 4 - Shift + '3'
    '$',  // 5 - Shift + '4'
    '%',  // 6 - Shift + '5'
    '&',  // 7 - Shift + '6'
    '/',  // 8 - Shift + '7'
    '(',  // 9 - Shift + '8'
    ')',  // 10 - Shift + '9'
    '=',  // 11 - Shift + '0'
    '?',  // 12 - Shift + 'ß'
    '`',  // 13 - Shift + '´'
    '\b', // 14 - Backspace
    '\t', // 15 - Tab
    'Q',  // 16
    'W',  // 17
    'E',  // 18
    'R',  // 19
    'T',  // 20
    'Z',  // 21
    'U',  // 22
    'I',  // 23
    'O',  // 24
    'P',  // 25
    220,  // 26 - 'Ü' (ASCII 220)
    '*',  // 27 - Shift + '+'
    '\n', // 28 - Enter
    0,	  // 29 - Left Strg
    'A',  // 30
    'S',  // 31
    'D',  // 32
    'F',  // 33
    'G',  // 34
    'H',  // 35
    'J',  // 36
    'K',  // 37
    'L',  // 38
    214,  // 39 - 'Ö' (ASCII 214)
    196,  // 40 - 'Ä' (ASCII 196)
    '^',  // 41 - Shift + '^'
    0,	  // 42 - Left Shift
    '>',  // 43 - Shift + '<'
    'Y',  // 44
    'X',  // 45
    'C',  // 46
    'V',  // 47
    'B',  // 48
    'N',  // 49
    'M',  // 50
    ';',  // 51 - Shift + ','
    ':',  // 52 - Shift + '.'
    '_',  // 53 - Shift + '-'
    0,	  // 54 - Right Shift
    0,	  // 55 - '*'-Key (Number)
    0,	  // 56 - Alt
    ' ',  // 57 - Space
    0	  // 58 - Caps Lock
};

const static uint8_t qwertz_altgr[] = {
    0,	  // 0
    0,	  // 1
    0,	  // 2 - '1' Key
    178,  // 3 - '²' (AltGr + '2')
    179,  // 4 - '³' (AltGr + '3')
    0,	  // 5 - '4' Key
    0,	  // 6 - '5' Key
    0,	  // 7 - '6' Key
    '{',  // 8 - '{' (AltGr + '7')
    '[',  // 9 - '[' (AltGr + '8')
    ']',  // 10 - ']' (AltGr + '9')
    '}',  // 11 - '}' (AltGr + '0')
    '\\', // 12 - '\' (AltGr + 'ß')
    0,	  // 13 - '´' Key
    0,	  // 14 - Backspace
    0,	  // 15 - Tab
    '@',  // 16 - '@' (AltGr + 'q')
    0,	  // 17 - 'w' Key
    164,  // 18 - '€' (AltGr + 'e')
    0,	  // 19 - 'r' Key
    0,	  // 20 - 't' Key
    0,	  // 21 - 'z' Key
    0,	  // 22 - 'u' Key
    0,	  // 23 - 'i' Key
    0,	  // 24 - 'o' Key
    0,	  // 25 - 'p' Key
    0,	  // 26 - 'ü' Key
    '~',  // 27 - '~' (AltGr + '+')
    0,	  // 28 - Enter
    0,	  // 29 - Linke Strg-Key
    0,	  // 30 - 'a' Key
    0,	  // 31 - 's' Key
    0,	  // 32 - 'd' Key
    0,	  // 33 - 'f' Key
    0,	  // 34 - 'g' Key
    0,	  // 35 - 'h' Key
    0,	  // 36 - 'j' Key
    0,	  // 37 - 'k' Key
    0,	  // 38 - 'l' Key
    0,	  // 39 - 'ö' Key
    0,	  // 40 - 'ä' Key
    0,	  // 41 - '^' Key
    0,	  // 42 - Linke Shift-Key
    '|',  // 43 - '|' (AltGr + '<')
    0,	  // 44 - 'y' Key
    0,	  // 45 - 'x' Key
    169,  // 46 - '©' (AltGr + 'c') optional
    0,	  // 47 - 'v' Key
    0,	  // 48 - 'b' Key
    0,	  // 49 - 'n' Key
    181,  // 50 - 'µ' (AltGr + 'm')
    0,	  // 51 - ',' Key
    0,	  // 52 - '.' Key
    0,	  // 53 - '-' Key
    0,	  // 54 - Rechte Shift-Key
    0,	  // 55 - '*'-Key (Nummernblock)
    0,	  // 56 - Alt-Key
    0,	  // 57 - Leertaste
    0,	  // 58 - Caps Lock
};

void kbd_init(kbd_t* self)
{
	self->caps = false;
	self->caps_lock = false;
	self->alt_gr = false;
	idt_set(0x21, asm_irq1_keyboard, IDT_KERNEL_INT_GATE);
	return;
};

static void _process_keystroke(kbd_t* self, const uint8_t makecode, const uint8_t breakcode)
{
	switch (makecode) {
	case 1:
	case 29:
	case 56:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
	case 87:
	case 88:
		break;
	case KALTGR: {
		self->alt_gr = !self->alt_gr;
		break;
	};
	case KLEFT_SHIFT: {
		self->caps = !self->caps;
		break;
	};
	case KCAPS_LOCK: {
		if (breakcode == 0) {
			self->caps_lock = !self->caps_lock;
		};
		break;
	};
	default: {
		if (breakcode == 0) {
			if (self->caps || self->caps_lock) {
				printf("%c", qwertz_upper[makecode]);
			} else if (kbd.alt_gr) {
				printf("%c", qwertz_altgr[makecode]);
			} else {
				printf("%c", qwertz_lower[makecode]);
			};
			break;
		};
		break;
	};
	};
	return;
};

void kbd_handler(void* dev, const uint8_t data)
{
	kbd_t* self = (kbd_t*)dev;
	const uint8_t ps2_package = data;
	const uint8_t mc = ps2_package & 0b01111111;
	const uint8_t bc = ps2_package & 0b10000000; // state can be released == 128 or pressed == 0
	_process_keystroke(self, mc, bc);
	return;
};

uint8_t kbd_translate(const uint8_t scancode)
{
	static bool shift = false;
	static bool caps_lock = false;

	switch (scancode) {
	case 0x2A:
	case 0x36:
		shift = true;
		return 0;
	case 0xAA:
	case 0xB6:
		shift = false;
		return 0;
	case 0x3A:
		caps_lock = !caps_lock;
		return 0;
	};
	if (scancode & 0x80) {
		return 0;
	}
	uint8_t ascii;

	if (shift) {
		ascii = qwertz_upper[scancode];
	} else if (caps_lock && (scancode >= 16 && scancode <= 40)) {
		ascii = qwertz_upper[scancode];
	} else {
		ascii = qwertz_lower[scancode];
	};
	return ascii;
}