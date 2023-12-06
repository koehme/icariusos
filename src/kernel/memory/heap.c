/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "heap.h"

HeapDescriptor kheap_descriptor;
Heap kheap;

/**
 * @brief Initializes a heap structure using the provided parameters.
 * @param self A pointer to the Heap structure to be initialized.
 * @param descriptor A pointer to the HeapDescriptor structure to store heap information.
 * @param heap_saddress The starting address of the heap data pool.
 * @param descriptor_saddress The starting address of the heap descriptor pool.
 * @param n_bytes The total size of the heap data pool.
 * @param block_size The size of each block in the heap data pool.
 */
void heap_init(Heap *self, HeapDescriptor *descriptor, void *heap_saddress, void *descriptor_saddress, const size_t n_bytes, const size_t block_size)
{
    // Initialize the heap descriptor
    descriptor->saddress = descriptor_saddress;
    // Assume block_size is a power of 2^12 (e.g., 4096)
    descriptor->total_descriptors = n_bytes / block_size; // 25600 total descriptors
    // Initialize the heap data pool
    self->descriptor = descriptor;
    self->saddress = heap_saddress;
    return;
};

/**
 * @brief Aligns a given number of bytes to the next multiple of the block size.
 * Given a heap object and a size in bytes, this function calculates the lower boundary
 * and aligns it to the next multiple of the block size specified in the heap object.
 * If a user wants to allocate 50 bytes, the aligned size will be 4096 bytes,
 * ensuring efficient block alignment. Similarly, for 5000 bytes, the user gets 8192 bytes.
 * The alignment is calculated as 'size = size - (size % block_size)' or 'size = size - (size % 4096)'.
 * The result is the next lower bound of the block_size, effectively aligning to 4096 bytes.
 * @param self A pointer to the Heap object.
 * @param n_bytes The number of bytes to align.
 * @return The aligned bytes.
 */
static size_t heap_align_bytes_to_block_size(Heap *self, const size_t n_bytes)
{
    const size_t lower_boundary = n_bytes - (n_bytes % self->block_size);
    const size_t aligned_size = lower_boundary + self->block_size;
    return aligned_size;
};

/**
 * @brief Allocates a block of memory from the heap.
 * Given a heap object and the desired number of bytes, this function allocates
 * a block of memory from the heap, ensuring alignment and managing the heap descriptor pool.
 * @param self A pointer to the Heap object.
 * @param n_bytes The number of bytes to allocate.
 * @return A pointer to the allocated memory block or 0x0 if allocation fails.
 */
void *heap_malloc(Heap *self, const size_t n_bytes)
{
    const size_t aligned_size = heap_align_bytes_to_block_size(self, n_bytes);
    // TODO: Implement the logic to search the heap descriptor pool for a free block
    // and check if enough blocks are available for allocation.
    // Also, determine the needed total blocks and proceed accordingly.
    // Return a pointer to the allocated memory block or 0x0 if allocation fails.
    return 0x0;
};