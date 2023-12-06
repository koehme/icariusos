/**
 * @file io.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IO_H
#define IO_H

typedef enum PICPorts
{
    PIC_1_CTRL = 0x20,
    PIC_1_DATA = 0x21,
    PIC_2_CTRL = 0xA0,
    PIC_2_DATA = 0xA1,
    PIC_ACK = 0x20
} PICPorts;

void asm_outb(unsigned short port, unsigned char value);
void asm_outw(unsigned short port, unsigned short value);

unsigned char asm_inb(unsigned short port);
unsigned short asm_inw(unsigned short port);

#endif