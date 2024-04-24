/**
 * @file io.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IO_H
#define IO_H

unsigned char inb(unsigned short port);
unsigned short inw(unsigned short port);
unsigned int inl(unsigned short port);

void outb(unsigned short port, unsigned char value);
void outw(unsigned short port, unsigned short value);
void outl(unsigned short port, unsigned int value);

#endif