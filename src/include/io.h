/**
 * @file io.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IO_H
#define IO_H

void outb(unsigned short port, unsigned char value);
void outw(unsigned short port, unsigned short value);

unsigned char inb(unsigned short port);
unsigned short inw(unsigned short port);

#endif