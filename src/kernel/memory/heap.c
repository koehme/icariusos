/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "heap.h"
#include "mem.h"
#include "icarius.h"

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
    descriptor->total_descriptors = n_bytes / block_size; // 25600 total descriptors - assume block_size is a power of 2^12 (e.g., 4096)
    // Initialize the heap data pool
    self->descriptor = descriptor;
    self->saddress = heap_saddress;
    self->block_size = block_size;
    // Initialize the descriptor table with 0x0
    mset8(descriptor->saddress, 0x0, descriptor->total_descriptors * sizeof(uint8_t));
    // Initialize the heap data pool with 0x0
    mset8(self->saddress, 0x0, n_bytes * sizeof(uint8_t));
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
    const size_t lower_aligned_boundary = n_bytes - (n_bytes % self->block_size);
    const size_t aligned_size = lower_aligned_boundary + self->block_size;
    return aligned_size;
};

static void *heap_search(Heap *self, const size_t blocks_needed, size_t start_block, size_t end_block)
{
    // Implement logic to search for a free block in the heap and allocate it.
    // Also, update descriptor flags and perform necessary bookkeeping.
    // Absolute address is the user's address

    size_t contiguous_free_blocks = 0;
    // Go through all descriptors and search for a matching free block
    for (size_t i = 0; i < self->descriptor->total_descriptors; i++)
    {
        const uint8_t curr_descriptor = self->descriptor->saddress[i];
        const bool is_block_free = !(curr_descriptor & DESCRIPTOR_IS_USED);

        if (is_block_free)
        {
            contiguous_free_blocks++;
        }
        else
        {
            contiguous_free_blocks = 0;
        };

        if (contiguous_free_blocks == blocks_needed)
        {
            // Mark all blocks from start to end with ALLOC_DESCRIPTOR_IS_USED
            // Mark the start block with additional ALLOC_DESCRIPTOR_IS_HEAD
            // Mark also the start block and all additional blocks except the end_block with ALLOC_DESCRIPTOR_HAS_NEXT
        };
    };
    void *absolute_data_pool_address = 0x0;
    // Todo Calculate the absolute_address with start_block and self->s_address
    return absolute_data_pool_address;
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
    const size_t n_bytes_aligned = heap_align_bytes_to_block_size(self, n_bytes);
    // Calculate the number of blocks needed for the allocation.
    // For example the n_bytes_aligned is 8192 / 4096 = 2 blocks in total we need for the allocation
    const size_t blocks_needed = n_bytes_aligned / self->block_size;

    if (blocks_needed > self->descriptor->total_descriptors)
    {
        kpanic("Exhausted heap descriptor pool; no additional blocks available for allocation.");
    };
    // Calculate the absolute address in the data pool with the following equation
    // heap_data_pool_start_address + (blocks_needed * block_size)
    size_t start_block = 0;
    size_t end_block = 0;

    void *free_block = heap_search(self, blocks_needed, start_block, end_block);

    if (free_block == 0x0)
    {
        kpanic("Exhausted heap pool; no additional blocks available for allocation.");
    };
    return free_block;
};