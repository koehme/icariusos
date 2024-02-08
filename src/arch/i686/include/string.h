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
char *itoa(uint32_t num, char *str, int32_t base);
int32_t kprintf(const char *fmt, ...);
char *scat(char *dest, const char *src);
char *sptr(char *str, const char c);
bool scmp(const char *s1, const char *s2);

#endif