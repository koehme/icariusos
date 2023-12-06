/**
 * @file mem.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void *mset8(void *dest, const uint8_t value, size_t n_bytes);
void *mset16(void *dest, const uint16_t value, size_t n_bytes);
void *mcpy(void *dest, const void *src, size_t n_bytes);

#endif