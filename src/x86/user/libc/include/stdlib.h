#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

int atoi(const char* str);
void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void heap_dump(void);
void dtoa(double n, char* res, int afterpoint);

#endif