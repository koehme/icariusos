/**
 * @file mouse.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

#include "kernel.h"

typedef enum MouseByteCycle
{
    MOUSE_BYTE_1 = 0,
    MOUSE_BYTE_2 = 1,
    MOUSE_BYTE_3 = 2,
} MouseByteCycle;

typedef struct Mouse
{
    bool enabled;
    uint8_t rel_x;
    uint8_t rel_y;
    MouseByteCycle cycle;
    union
    {
        struct
        {
            uint8_t byte0, byte1, byte2, byte3, byte4;
        };
        uint8_t bytes[5];
    };
} Mouse;

typedef enum MousePort
{
    MOUSE_DATA_PORT = 0x60,
    MOUSE_STATUS_PORT = 0x64,
} MousePort;

typedef enum MouseBufferType
{
    MOUSE_OUTPUT_BUFFER = 0x0,
    MOUSE_INPUT_BUFFER = 0x1,
} MouseBufferType;

typedef enum MouseCommand
{
    MOUSE_ENABLE_AUX = 0xA8,
    MOUSE_DISABLE = 0xA7,
    MOUSE_GET_COMPAQ_STATUS = 0x20,
    MOUSE_SET_COMPAQ_STATUS = 0x60,
    MOUSE_SEND_COMMAND = 0xD4,
    MOUSE_ENABLE_DATA_REPORT = 0xF4,
    MOUSE_SET_DEFAULT = 0xF6,
    MOUSE_DETECT_ACK = 0xFA,
} MouseCommand;

void mouse_handler(Mouse *self);
void mouse_init(Mouse *self);

#endif