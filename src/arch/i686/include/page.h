/**
 * @file page.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PAGE_H
#define PAGE_H

#include <stddef.h>
#include <stdint.h>

extern void asm_page_enable(void);

typedef uint32_t PageEntry;

/**
 * @brief Representing the translation from virtual address to page.
 */
typedef struct PageVirtual
{
    uint32_t pd_index; // Page Directory Index
    uint32_t pt_index; // Page Table Index
} PageVirtual;

typedef enum PageFlags
{
    PAGE_PRESENT = 0b00000001,         // Set if the page is present in memory.
    PAGE_READ_WRITE = 0b00000010,      // If set, the page is writeable; if unset, the page is read-only.
    PAGE_USER_SUPERVISOR = 0b00000100, // If set, it's a user-mode page; else, it's a supervisor (kernel)-mode page.
    PAGE_RESERVED = 0b00001000,        // Used by the CPU internally and cannot be trampled.
    PAGE_ACCESSED = 0b00010000,        // Set if the page has been accessed (set by the CPU).
    PAGE_DIRTY = 0b00100000,           // Set if the page has been written to (dirty).
    PAGE_AVAIL = 0b11000000,           // Unused and available for kernel use (3 bits).
    PAGE_SIZE = 4096,                  // Size of a page in bytes.
    PAGE_TABLE_ENTRIES = 1024,         // Number of entries in a page table.
    PAGE_MASK = 0b1111111111,          // Mask to extract the page table index from a virtual address.
} PageFlags;

typedef struct PageDirectory
{
    uint32_t *directory;
} PageDirectory;

void page_init_directory(PageDirectory *self, uint8_t flags);
void page_switch(PageEntry *dir);

PageVirtual virt_address_get_pd_indicies(void *virt_addr);
int32_t virt_address_map(PageEntry *dir, void *virt_addr, const uint32_t phy_addr);

#endif