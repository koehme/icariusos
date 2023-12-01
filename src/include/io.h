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

void outb(unsigned short port, unsigned char value);
void outw(unsigned short port, unsigned short value);

unsigned char inb(unsigned short port);
unsigned short inw(unsigned short port);

#endif