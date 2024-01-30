/**
 * @file string.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdbool.h>

#include "status.h"
#include "kernel.h"

bool is_alpha(const char c);
size_t slen(const char *str);
char *itoa(uint32_t num, char *str, int base);
int kprintf(const char *fmt, ...);
char *scat(char *dest, const char *src);
bool scmp(const char *s1, const char *s2);

#endif