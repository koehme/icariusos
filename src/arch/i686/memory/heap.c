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

HeapTable heap_table = {
    .saddr = 0x0,
    .total_blocks = 0,
};

Heap heap = {
    .table = 0x0,
    .saddr = 0x0,
    .block_size = 0,
};

// Initializes a heap
void heap_init(Heap *self, HeapTable *table, void *heap_saddr, void *table_saddr, const size_t n_bytes, const size_t block_size)
{
    // Initialize the heap table
    table->saddr = table_saddr;
    table->total_blocks = n_bytes / block_size; // 25600 total descriptors - assume block_size is a power of 2^12 (e.g., 4096)
    // Initialize the heap data structure
    self->table = table;
    self->saddr = heap_saddr;
    self->block_size = block_size;
    mset8(table->saddr, 0x0, table->total_blocks * sizeof(uint8_t));
    return;
};

static size_t heap_align_bytes_to_block_size(Heap *self, const size_t n_bytes)
{
    const size_t lower_aligned_boundary = n_bytes - (n_bytes % self->block_size);
    const size_t aligned_size = lower_aligned_boundary + self->block_size;
    return aligned_size;
};

static void heap_mark_as_used(Heap *self, const size_t start_block, const size_t end_block, const size_t blocks_needed)
{
    // Mark the first block in the heap table_entry as head, next and used
    uint8_t table_entry = 0b00000000;
    // should be 65 0b01000001 BIT_FLAG_USED and BIT_FLAG_HEAD
    table_entry |= HEAP_TABLE_FLAG_USED;
    table_entry |= HEAP_TABLE_FLAG_HEAD;

    if (blocks_needed > 1)
    {
        table_entry |= HEAP_TABLE_FLAG_NEXT; // should be 193 IS_USED and IS_HEAD and HAS_NEXT
    };

    for (size_t curr_block = start_block; curr_block <= end_block; curr_block++)
    {
        // Set the descriptor for the current block
        self->table->saddr[curr_block] = table_entry;
        // Reset descriptor for a clean state
        table_entry = 0b00000000;
        // Mark the block as HEAP_TABLE_FLAG_USED
        table_entry |= HEAP_TABLE_FLAG_USED;
        // If the current block is not the last one, mark it as HEAP_TABLE_FLAG_HAS_NEXT AND HEAP_TABLE_FLAG_USED == 129
        const bool has_next_block = (curr_block < end_block - 1);

        if (has_next_block)
        {
            table_entry |= HEAP_TABLE_FLAG_NEXT; // Mark block as HEAP_TABLE_FLAG_HAS_NEXT AND HEAP_TABLE_FLAG_USED == 129
        };
    };
    return;
};

static int32_t heap_mark_as_free(Heap *self, const size_t start_block)
{
    uint8_t table_entry = self->table->saddr[start_block];
    const bool is_head = table_entry & HEAP_TABLE_BLOCK_IS_HEAD;

    if (!is_head)
    {
        kprintf("HeapError: Wrong start block or miscalculated alignment.");
        return -1;
    };
    bool has_next_block = table_entry & HEAP_TABLE_BLOCK_HAS_NEXT;
    size_t curr_block = start_block;
    self->table->saddr[curr_block] = 0b00000000;

    while (has_next_block)
    {
        curr_block++;
        table_entry = self->table->saddr[curr_block];
        has_next_block = table_entry & HEAP_TABLE_BLOCK_HAS_NEXT;
        self->table->saddr[curr_block] = 0b00000000;
    };
    return 1;
};

static size_t heap_search(Heap *self, const size_t blocks_needed)
{
    size_t start_block = -1;
    size_t contiguous_free_blocks = 0;

    // Go through all table entries and search for a matching free block
    for (size_t i = 0; i < self->table->total_blocks; i++)
    {
        const uint8_t curr_table_entry = self->table->saddr[i];
        const bool is_block_free = !(curr_table_entry & HEAP_TABLE_BLOCK_IS_USED);

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

void *heap_malloc(Heap *self, const size_t n_bytes)
{
    // Number of blocks needed for the allocation
    const size_t n_bytes_aligned = heap_align_bytes_to_block_size(self, n_bytes);
    // Blocks in total we need for the allocation
    const size_t blocks_needed = n_bytes_aligned / self->block_size;

    if (blocks_needed > self->table->total_blocks)
    {
        kpanic("HeapError: No additional blocks available for allocation\n");
    };
    size_t block = -1;
    block = heap_search(self, blocks_needed);

    if (block == -1)
    {
        kpanic("HeapError: No contiguous blocks available for allocation\n");
        return 0x0;
    };
    void *absolute_address = 0x0;
    absolute_address = self->saddr + (block * self->block_size);
    return absolute_address;
};

void heap_free(Heap *self, void *ptr)
{
    if (ptr == 0x0)
    {
        return;
    };
    const size_t start_block = (((size_t)ptr) / self->block_size) % self->block_size;
    const int32_t res = heap_mark_as_free(self, start_block);

    if (res == -1)
    {
        kpanic("HeapError: Failed to deallocate memory\n");
        return;
    };
    return;
};

size_t heap_table_get_used_blocks(const Heap *self)
{
    size_t used_blocks = 0;

    for (size_t i = 0; i < self->table->total_blocks; ++i)
    {
        const uint8_t table_entry = self->table->saddr[i];

        if (table_entry & HEAP_TABLE_FLAG_USED)
        {
            used_blocks++;
        };
    };
    return used_blocks;
};