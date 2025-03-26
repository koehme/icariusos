#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>

int puts(const char* s);
int printf(const char* restrict format, ...);
int vsnprintf(char* buffer, size_t size, const char* format, va_list args);

#endif