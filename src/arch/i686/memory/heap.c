/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "heap.h"
#include "mem.h"
#include "kernel.h"
#include "string.h"

HeapDescriptor kheap_descriptor = {
    .saddress = 0x0,
    .total_descriptors = 0,
};

Heap kheap = {
    .descriptor = 0x0,
    .saddress = 0x0,
    .block_size = 0,
};

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
    mset8(descriptor->saddress, 0x0, descriptor->total_descriptors * sizeof(uint8_t));
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

/**
 * @brief Marks a range of blocks in the heap as used and updates descriptors.
 * Given a heap object and a range of block indices, marks the corresponding
 * blocks as used in the heap descriptor pool. The first block is marked as the head with
 * next and used flags, and the last block is marked as used with no next flag to indicate
 * the end of the allocation.
 * @param self A pointer to the Heap object.
 * @param start_block The index of the first block to mark.
 * @param end_block The index of the last block to mark.
 * @param blocks_needed The number of blocks needed for marking HAS_NEXT.
 */
static void heap_mark_as_used(Heap *self, const size_t start_block, const size_t end_block, const size_t blocks_needed)
{
    const uint8_t BIT_FLAG_USED = (1u << 0);
    const uint8_t BIT_FLAG_HEAD = (1u << 6);
    const uint8_t BIT_FLAG_HAS_NEXT = (1u << 7);
    // Mark the first block in the heap descriptor as head, next and used
    uint8_t descriptor = 0b00000000;
    // should be 65 0b01000001 BIT_FLAG_USED and BIT_FLAG_HEAD
    descriptor |= BIT_FLAG_USED;
    descriptor |= BIT_FLAG_HEAD;

    if (blocks_needed > 1)
    {
        descriptor |= BIT_FLAG_HAS_NEXT; // should be 193 IS_USED and IS_HEAD and HAS_NEXT
    };

    for (size_t curr_block = start_block; curr_block <= end_block; curr_block++)
    {
        // Set the descriptor for the current block
        self->descriptor->saddress[curr_block] = descriptor;
        // Reset descriptor for a clean state
        descriptor = 0b00000000;
        // Mark the block as BIT_FLAG_USED
        descriptor |= BIT_FLAG_USED;
        // If the current block is not the last one, mark it as BIT_FLAG_HAS_NEXT AND BIT_FLAG_USED == 129
        const bool has_next_block = (curr_block < end_block - 1);

        if (has_next_block)
        {
            descriptor |= BIT_FLAG_HAS_NEXT; // Mark block as BIT_FLAG_HAS_NEXT AND BIT_FLAG_USED == 129
        };
    };
    return;
};

/**
 * @brief Marks a block in the heap as free.
 * It checks if the block is a head block, indicating a valid starting point. If successful, it updates the block descriptors and returns 1.
 * @param self A pointer to the Heap structure.
 * @param start_block The index of the block to mark as free.
 * @return 1 on success, -1 on failure.
 */
static int32_t heap_mark_as_free(Heap *self, const size_t start_block)
{
    uint8_t descriptor = self->descriptor->saddress[start_block];
    const bool is_head = descriptor & DESCRIPTOR_IS_HEAD;

    if (!is_head)
    {
        kprintf("Oops. Something went wrong. Maybe a wrong start_block or miscalculated alignment.");
        return -1;
    };
    bool has_next_block = descriptor & DESCRIPTOR_HAS_NEXT;
    size_t curr_block = start_block;
    self->descriptor->saddress[curr_block] = 0b00000000;

    while (has_next_block)
    {
        curr_block++;
        descriptor = self->descriptor->saddress[curr_block];
        has_next_block = descriptor & DESCRIPTOR_HAS_NEXT;
        self->descriptor->saddress[curr_block] = 0b00000000;
    };
    return 1;
};

/**
 * @brief Searches for a contiguous block of free memory in the heap.
 * Given a heap object and the number of contiguous blocks needed, this function
 * searches for a suitable block in the heap descriptor pool. It returns the index
 * of the first block in the found contiguous free memory block and marks the blocks
 * as used in the heap descriptor pool.
 * @param self A pointer to the Heap object.
 * @param blocks_needed The number of contiguous blocks needed.
 * @return The index of the first block in the allocated memory, or SIZE_MAX if no
 * contiguous block is found.
 */
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
            // Set the start block
            if (start_block == -1)
            {
                start_block = i;
            };
            // Add a free block to the sum variable
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
    heap_mark_as_used(self, start_block, end_block, blocks_needed);
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

void heap_free(Heap *self, void *ptr)
{
    if (ptr == 0x0)
    {
        return;
    };
    /*
    In our understanding, the 'ptr' variable represents the absolute address in the heap data memory.
    For instance, consider the following code snippet:

    void* ptr = kmalloc(1024); // Allocating a block of 1024 bytes (1 block, as it is below 4096)

    kfree(ptr);

    Since this is the initial allocation, the pointer to be freed should be 0x01000000,
    and it is expected to be the first block in the heap.

    Let's examine another example:
    void* ptr2 = kmalloc(4096); // Allocating a block of 4096 bytes (2 blocks required)
    kfree(ptr2);
    For this case, ptr2 should be 0x01001000, and we utilize the modulo operator to determine the start block.

                        0x01001000 / 4096
    Another example is (16781312 / 4096) % 4096
                                  ^
                                 4097     % 4096 = 1
    Thus, the start offset for ptr2 should be 1.

    Subsequently, we iterate through the heap descriptor, checking if the IS_HEAD flag is set.
    If the flag is set, we continue the loop and examine if HAS_NEXT is also set, except for the last block
    where HAS_NEXT should not be set.
    */
    const size_t start_block = (((size_t)ptr) / self->block_size) % self->block_size;
    const int32_t res = heap_mark_as_free(self, start_block);

    if (res == -1)
    {
        kpanic("Error: heap_free failed to deallocate memory.");
        return;
    };
    return;
};