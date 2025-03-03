#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* s);
void* memcpy(void* dest, const void* src, size_t n);

#endif