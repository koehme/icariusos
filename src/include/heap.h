/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pfa.h"

#define CHUNK_SIZE 4096

#define KERNEL_HEAP_START 0xC1000000
#define KERNEL_HEAP_MAX 0xC2BFFFFF

typedef struct heap heap_t;
typedef struct heap_block heap_block_t;

void heap_init(heap_t* self);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);
void heap_dump(const heap_t* self);
void heap_trace(const heap_t* self);

#endif