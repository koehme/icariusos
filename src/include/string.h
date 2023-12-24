/**
 * @file string.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdbool.h>

bool is_alpha(const char c);
size_t slen(const char *str);

#endif