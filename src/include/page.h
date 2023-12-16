/**
 * @file page.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

typedef uint32_t PageEntry;

typedef enum
{
    PAGE_PRESENT = 0b00000001,         // Set if the page is present in memory.
    PAGE_READ_WRITE = 0b00000010,      // If set, the page is writeable; if unset, the page is read-only.
    PAGE_USER_SUPERVISOR = 0b00000100, // If set, it's a user-mode page; else, it's a supervisor (kernel)-mode page.
    PAGE_RESERVED = 0b00001000,        // Used by the CPU internally and cannot be trampled.
    PAGE_ACCESSED = 0b00010000,        // Set if the page has been accessed (set by the CPU).
    PAGE_DIRTY = 0b00100000,           // Set if the page has been written to (dirty).
    PAGE_AVAIL = 0b11000000,           // Unused and available for kernel use (3 bits).
    PAGE_SIZE = 4096,
    PAGE_TABLE_ENTRIES = 1024,
} PageFlags;

typedef struct PageDirectory
{
    uint32_t *directory;
} PageDirectory;

PageDirectory *page_init_directory(const uint8_t flags);

#endif