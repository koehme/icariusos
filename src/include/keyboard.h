/**
 * @file kbd.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef enum Keyboard_Controller_Status_Mask
{
    KEYBOARD_CTRL_STATS_MASK_OUT_BUF = 0x1,  // 00000001
    KEYBOARD_CTRL_STATS_MASK_IN_BUF = 0x2,   // 00000010
    KEYBOARD_CTRL_STATS_MASK_SYSTEM = 0x4,   // 00000100
    KEYBOARD_CTRL_STATS_MASK_CMD_DATA = 0x8, // 00001000
    KEYBOARD_CTRL_STATS_MASK_LOCKED = 0x10,  // 00010000
    KEYBOARD_CTRL_STATS_MASK_AUX_BUF = 0x20, // 00100000
    KEYBOARD_CTRL_STATS_MASK_TIMEOUT = 0x40, // 01000000
    KEYBOARD_CTRL_STATS_MASK_PARITY = 0x80   // 10000000
} Keyboard_Controller_Status_Mask;

typedef enum Keyboard_Encoder_Ports
{
    KEYBOARD_ENC_INPUT_BUF = 0x60,
    KEYBOARD_ENC_CMD_REG = 0x60
} Keyboard_Encoder_Ports;

typedef enum Keyboard_Controller_Ports
{
    KEYBOARD_CTRL_STATS_REG = 0x64,
    KEYBOARD_CTRL_CMD_REG = 0x64
} Keyboard_Controller_Ports;

typedef enum Keyboard_Controller_Commands
{
    KEYBOARD_CTRL_SELF_TEST = 0xAA, // Self-Test
    KEYBOARD_CTRL_ENABLE = 0xAE,    // Enable Keyboard
    KEYBOARD_CTRL_DISABLE = 0xAD,   // Disable Keyboard
} Keyboard_Controller_Commands;

typedef struct Keyboard
{
    bool enabled;
    bool caps;
    bool caps_lock;
    bool alt_gr;
} Keyboard;

void keyboard_init();
void keyboard_read(Keyboard *self);
int keyboard_wait(void);

#endif