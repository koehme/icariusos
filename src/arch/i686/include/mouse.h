/**
 * @file mouse.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Mouse
{
    bool enabled;
    int32_t x;
    int32_t y;
} Mouse;

typedef enum MousePort
{
    PS2_DATA_PORT = 0x60,
    PS2_CTRL_PORT = 0x64,
} MousePort;

typedef enum MouseCommand
{
    MOUSE_ENABLE = 0xA8,
    MOUSE_DISABLE = 0xA7,
    MOUSE_READ = 0x20,
    MOUSE_WRITE = 0x60,
} MouseCommand;

void mouse_init(void);

#endif