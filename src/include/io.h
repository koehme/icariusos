/**
 * @file io.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IO_H
#define IO_H

// Programmable Interrupt Controller
#define PIC_1_CTRL 0x20 // Primary PIC control register
#define PIC_1_DATA 0x21 // Primary PIC data register
#define PIC_2_CTRL 0xA0 // Secondary PIC control register
#define PIC_2_DATA 0xA1 // Secondary PIC data register
#define PIC_ACK 0x20    // Acknowledge-Bit (0x20)

void asm_outb(unsigned short port, unsigned char value);
void asm_outw(unsigned short port, unsigned short value);

unsigned char asm_inb(unsigned short port);
unsigned short asm_inw(unsigned short port);

#endif