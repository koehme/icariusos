/**
 * @file io.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IO_H
#define IO_H

unsigned char asm_inb(unsigned short port);
unsigned short asm_inw(unsigned short port);
unsigned int asm_inl(unsigned short port);

void asm_outb(unsigned short port, unsigned char value);
void asm_outw(unsigned short port, unsigned short value);
void asm_outl(unsigned short port, unsigned int value);

#endif