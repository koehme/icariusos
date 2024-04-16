/**
 * @file mouse.h
 * @author Kevin Oehme
 * @copyright MIT
 */

typedef enum MousePort
{
    MOUSE_DATA = 0x60,
    MOUSE_STATUS = 0x64
} MousePort;

typedef enum MouseCommand
{
    MOUSE_ENABLE_INTERRUPTS = 0xA8,
    MOUSE_DISABLE_INTERRUPTS = 0xA7,
    MOUSE_READ = 0x20,
    MOUSE_WRITE = 0x60
} MouseCommand;

void mouse_init(void);