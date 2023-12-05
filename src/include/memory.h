/**
 * @file memory.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void *mset(void *dest, const int value, const size_t n_bytes);
void *mcpy(void *dest, const void *src, size_t n_bytes);

#endif