/**
 * @file string.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>

#include "errno.h"
#include "kernel.h"

char* strncpy(char* dest, const char* src, size_t n);
size_t strlen(const char* str);
bool is_alpha(const char c);
size_t slen(const char* str);

char* scat(char* dest, const char* src);
char* sptr(char* str, const char c);
bool scmp(const char* s1, const char* s2);

void* memset(void* ptr, int value, size_t num);
void* memset16(void* ptr, uint16_t value, size_t num);
void* memcpy(void* dest, const void* src, size_t n);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
void* memmove(void* dest, const void* src, size_t count);

#endif