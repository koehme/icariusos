/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

typedef enum HeapDescriptorFlags
{
    ALLOC_DESCRIPTOR_IS_FREE = 0b00000000, // 0
    ALLOC_DESCRIPTOR_IS_USED = 0b00000001, // 1
    ALLOC_DESCRIPTOR_IS_HEAD = 0b0100000,  // 64
    ALLOC_DESCRIPTOR_HAS_NEXT = 0b10000000 // 128
} HeapDescriptorFlags;

typedef struct HeapDescriptor
{
    uint8_t *saddress;
    size_t total_descriptors;
} HeapDescriptor;

typedef struct Heap
{
    HeapDescriptor *descriptor;
    void *saddress;
    size_t block_size;
} Heap;

void heap_init(Heap *self, HeapDescriptor *descriptor, void *heap_saddress, void *descriptor_saddress, const size_t n_bytes, const size_t block_size);
void *heap_malloc(Heap *self, const size_t n_bytes);

#endif