#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* s);
void* memcpy(void* dest, const void* src, size_t n);
int strcmp(const char* s1, const char* s2);
void* memset(void* s, int c, size_t n);
char* strncpy(char* dest, const char* src, size_t n);

#endif