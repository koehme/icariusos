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
    DESCRIPTOR_IS_FREE = 0b00000000,
    DESCRIPTOR_IS_USED = 0b00000001,
    DESCRIPTOR_IS_HEAD = 0b01000000,
    DESCRIPTOR_HAS_NEXT = 0b10000000
} HeapDescriptorFlags;

/**
 * @brief Representing a descriptor for the heap.
 */
typedef struct HeapDescriptor
{
    uint8_t *saddress;        // Start address of the descriptor table
    size_t total_descriptors; // Total number of descriptors in the table
} HeapDescriptor;

/**
 * @brief Structure representing a gigant piece of memory called a heap.
 *
 * A heap is a dynamic memory allocation structure that allows flexible and
 * efficient management of memory during runtime. The Heap structure encapsulates
 * essential components for dynamic memory allocation in a kernel, including a
 * descriptor table for tracking the state of memory blocks and a data pool
 * for storing variable-sized memory allocations.
 *
 * The need for a heap arises due to the dynamic and unpredictable nature of
 * memory requirements in a kernel environment. Unlike static memory allocation,
 * a heap allows the kernel to allocate and deallocate memory on-demand, providing
 * a mechanism for tasks such as storing data structures, managing processes, and
 * facilitating other dynamic operations without the constraints of fixed memory
 * sizes.
 */
typedef struct Heap
{
    HeapDescriptor *descriptor; // Pointer to the heap descriptor
    void *saddress;             // Start address of the heap data pool
    size_t block_size;          // Size of each memory block in the heap
} Heap;

void heap_init(Heap *self, HeapDescriptor *descriptor, void *heap_saddress, void *descriptor_saddress, const size_t n_bytes, const size_t block_size);
void *heap_malloc(Heap *self, const size_t n_bytes);
void heap_free(Heap *self, void *ptr);

#endif