/**
 * @file memory.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void *msetb(void *dest, const uint8_t value, size_t n_bytes);
void *msetw(void *dest, const uint16_t value, size_t n_bytes);
void *mcpy(void *dest, const void *src, size_t n_bytes);

#endif