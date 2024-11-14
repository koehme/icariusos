/**
 * @file kbd.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "kernel.h"

#define KEYBOARD_CTRL_STATS_MASK_OUT_BUF 0x1  // 00000001
#define KEYBOARD_CTRL_STATS_MASK_IN_BUF 0x2   // 00000010
#define KEYBOARD_CTRL_STATS_MASK_SYSTEM 0x4   // 00000100
#define KEYBOARD_CTRL_STATS_MASK_CMD_DATA 0x8 // 00001000
#define KEYBOARD_CTRL_STATS_MASK_LOCKED 0x10  // 00010000
#define KEYBOARD_CTRL_STATS_MASK_AUX_BUF 0x20 // 00100000
#define KEYBOARD_CTRL_STATS_MASK_TIMEOUT 0x40 // 01000000
#define KEYBOARD_CTRL_STATS_MASK_PARITY 0x80  // 10000000
// Keyboard Encoder Ports
#define KEYBOARD_ENC_INPUT_BUF 0x60
#define KEYBOARD_ENC_CMD_REG 0x60
// Keyboard Controller Ports
#define KEYBOARD_CTRL_STATS_REG 0x64
#define KEYBOARD_CTRL_CMD_REG 0x64
// Keyboard Controller Commands
#define KEYBOARD_CTRL_SELF_TEST 0xAA
#define KEYBOARD_CTRL_ENABLE 0xAE
#define KEYBOARD_CTRL_DISABLE 0xAD

#define KESC 0x01
#define KLEFT_SHIFT 0x2A
#define KRIGHT_SHIFT 0x36
#define KCAPS_LOCK 0x3A
#define KALTGR 0x60

typedef struct kbd_t {
	bool caps;
	bool caps_lock;
	bool alt_gr;
} kbd_t;

void kbd_init();
void kbd_handler(void* dev, const uint8_t data);

#endif