/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

typedef enum HeapTableBlockStatus
{
    HEAP_TABLE_BLOCK_IS_FREE = 0b00000000,
    HEAP_TABLE_BLOCK_IS_USED = 0b00000001,
    HEAP_TABLE_BLOCK_IS_HEAD = 0b01000000,
    HEAP_TABLE_BLOCK_HAS_NEXT = 0b10000000
} HeapTableBlockStatus;

typedef enum HeapTableBlockFlags
{
    HEAP_TABLE_FLAG_USED = (1u << 0),
    HEAP_TABLE_FLAG_HEAD = (1u << 6),
    HEAP_TABLE_FLAG_NEXT = (1u << 7)
} HeapTableBlockFlags;

typedef struct HeapTable
{
    uint8_t *saddr;      // Pointer to the location for the marked table allocation data structure
    size_t total_blocks; // 25600 total descriptors if heap_init called with n_bytes 1024 * 1024 * 100 - assume block_size is a power of 2^12 (e.g., 4096)
} HeapTable;

typedef struct Heap
{
    HeapTable *table;  // Pointer to the heap table data structure
    void *saddr;       // Start address of the raw heap data pool
    size_t block_size; // Size of each memory block in the heap
} Heap;

void heap_init(Heap *self, HeapTable *table, void *heap_saddress, void *descriptor_saddress, const size_t n_bytes, const size_t block_size);
void *heap_malloc(Heap *self, const size_t n_bytes);
void heap_free(Heap *self, void *ptr);
size_t heap_table_get_used_blocks(const Heap *self);

#endif