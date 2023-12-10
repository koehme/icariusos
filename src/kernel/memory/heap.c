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
    // DEBUGGING
    const size_t partial_init_size = 1024;
    // Initialize the descriptor table with 0x0
    // Replace partial_init_size * sizeof(uint8_t) with descriptor->total_descriptors * sizeof(uint8_t)
    mset8(descriptor->saddress, 0x0, partial_init_size);
    // Initialize the heap data pool with 0x0
    // Replace partial_init_size * sizeof(uint8_t) with n_bytes * sizeof(uint8_t)
    mset8(self->saddress, 0x0, partial_init_size);
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

static void heap_mark(Heap *self, size_t start_block, size_t end_block)
{
    uint8_t *head_descriptor = &self->descriptor->saddress[start_block];
    // Mark the first block in the heap descriptor as head, next and used
    *head_descriptor |= DESCRIPTOR_IS_HEAD | DESCRIPTOR_IS_USED | DESCRIPTOR_HAS_NEXT;

    for (size_t curr_block = start_block; curr_block <= end_block; curr_block++)
    {
        uint8_t *curr_descriptor = &self->descriptor->saddress[curr_block];

        if (curr_block == end_block)
        {
            // Mark the last block not with DESCRIPTOR_HAS_NEXT to indiciate here is the end of the allocation
            *curr_descriptor |= DESCRIPTOR_IS_USED;
            break;
        };
        *curr_descriptor |= DESCRIPTOR_IS_USED | DESCRIPTOR_HAS_NEXT;
    };
    return;
};

static size_t heap_search(Heap *self, const size_t blocks_needed)
{
    size_t start_block = -1;
    size_t contiguous_free_blocks = 0;

    // Go through all descriptors and search for a matching free block
    for (size_t i = 0; i < self->descriptor->total_descriptors; i++)
    {
        const uint8_t curr_descriptor = self->descriptor->saddress[i];
        const bool is_block_free = !(curr_descriptor & DESCRIPTOR_IS_USED);

        if (is_block_free)
        {
            if (start_block == -1)
            {
                start_block = i;
            };
            contiguous_free_blocks++;
        }
        else
        {
            start_block = -1;
            contiguous_free_blocks = 0;
            continue;
        };

        if (contiguous_free_blocks == blocks_needed)
        {
            break;
        };
    };

    if (start_block == -1)
    {
        return -1;
    };
    const size_t end_block = start_block + blocks_needed - 1;
    heap_mark(self, start_block, end_block);
    return start_block;
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
    size_t block = -1;
    block = heap_search(self, blocks_needed);

    if (block == -1)
    {
        kpanic("Fragmented heap descriptor pool; no contiguous blocks available for allocation.");
        return 0x0;
    };
    void *absolute_address = 0x0;
    absolute_address = self->saddress + (block * self->block_size);
    return absolute_address;
};