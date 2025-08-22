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

usize strlen(const char* str);
s32 strcmp(const char* str1, const char* str2);
char* strncpy(char* dest, const char* src, usize n);
bool isalpha(const char c);
char* scat(char* dest, const char* src);
char* strchr(char* str, const char c);
bool scmp(const char* s1, const char* s2);
void* memset(void* ptr, int value, usize num);
void* memset16(void* ptr, u16 value, usize num);
void* memcpy(void* dest, const void* src, usize n);
int memcmp(const void* ptr1, const void* ptr2, usize num);
void* memmove(void* dest, const void* src, usize count);