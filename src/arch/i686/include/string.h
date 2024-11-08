/**
 * @file string.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>

#include "kernel.h"
#include "status.h"

bool is_alpha(const char c);
size_t slen(const char* str);
char* itoa(int32_t num, char* str, int32_t base);
int32_t printf(const char* fmt, ...);
char* scat(char* dest, const char* src);
char* sptr(char* str, const char c);
bool scmp(const char* s1, const char* s2);

void* mset8(void* dest, const uint8_t value, size_t n_bytes);
void* mset16(void* dest, const uint16_t value, size_t n_bytes);
void* mcpy(void* dest, const void* src, size_t n_bytes);
int32_t mcmp(const void* s1, const void* s2, size_t n_bytes);
void* mmove(void* dest, const void* src, size_t n_bytes);

#endif