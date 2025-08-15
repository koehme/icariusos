/**
 * @file stdlib.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "stddef.h"

size_t slen(const char* str);
char* itoa(int value, char* str, int base);
void dtoa(double n, char* res, int afterpoint);
char* utoa(unsigned int num, char* str, int base);
char* utoa64(uint64_t v, char* out, int base);
void dtoa(double n, char* res, int afterpoint);
int atoi(const char* str);