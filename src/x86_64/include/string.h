/**
 * @file string.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "types.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

usize strlen(const ch* str);
s32 strcmp(const ch* str1, const ch* str2);
ch* strncpy(ch* dest, const ch* src, usize n);
b8 isalpha(const ch c);
ch* scat(ch* dest, const ch* src);
ch* strchr(ch* str, const ch c);
b8 scmp(const ch* s1, const ch* s2);
void* memset(void* ptr, s32 value, usize num);
void* memset16(void* ptr, u16 value, usize num);
void* memcpy(void* dest, const void* src, usize n);
s32 memcmp(const void* ptr1, const void* ptr2, usize num);
void* memmove(void* dest, const void* src, usize count);