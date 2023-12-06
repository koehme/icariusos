/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "heap.h"

HeapDescriptor kheap_descriptor;
Heap kheap;

void heap_init(Heap *self, HeapDescriptor *descriptor, void *heap_saddress, void *descriptor_saddress, const size_t n_bytes, const size_t align_at_bytes)
{
    // Assume align_at_bytes is a power of 2^12 (e.g., 4096)
    const size_t block_size = align_at_bytes;
    // Initialize the heap descriptor
    descriptor->saddress = descriptor_saddress;
    descriptor->total_descriptors = n_bytes / block_size; // 25600 total descriptors
    // Initialize the heap data pool
    self->descriptor = descriptor;
    self->saddress = heap_saddress;
    return;
};