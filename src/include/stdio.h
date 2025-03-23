/**
 * @file stdio.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STDIO_H
#define STDIO_H

#include "vbe.h"
#include <stdarg.h>
#include <stddef.h>

int printf(const char* format, ...);
int vprintf(const char* fmt, va_list args);

#endif