/**
 * @file kbd.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KBD_H
#define KBD_H

#include <stdint.h>

typedef enum KBDPorts
{
    KBD_DATA = 0x60,
    KBD_COMMAND = 0x64,
} KBDPorts;

void kbd_read();

#endif