/**
 * @file stdlib.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STDLIB_H
#define STDLIB_H

char* itoa(int value, char* str, int base);
void dtoa(double n, char* res, int afterpoint);
char* utoa(unsigned int num, char* str, int base);

#endif