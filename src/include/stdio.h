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

int kprintf(const char* format, ...);
int vkprintf(const char* fmt, va_list args);

#endif