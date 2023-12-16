/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "icarius.h"

/**
 * @brief Allocates memory for the page directory and page tables and initializes them.
 * @param flags The flags to be used for initialization.
 * @return A pointer to the initialized page directory structure.
 */
PageDirectory *page_init_directory(const uint8_t flags)
{
    // Allocate memory for the 4kb page directory
    PageEntry *directory = kcalloc(sizeof(PageEntry) * PAGE_TABLE_ENTRIES);
    size_t dir_offset = 0;
    // Loop through each entry in the page directory
    for (size_t dir_index = 0; dir_index < PAGE_TABLE_ENTRIES; dir_index++)
    {
        // Allocate memory for a page table
        PageEntry *entry = kcalloc(sizeof(PageEntry) * PAGE_TABLE_ENTRIES);
        // Loop through each entry in the page table
        for (size_t ptable_index = 0; ptable_index < PAGE_TABLE_ENTRIES; ptable_index++)
        {
            const size_t physical_addr = dir_offset + (ptable_index * PAGE_SIZE);
            // Set the page table entry to the physical address with flags
            // for example in the first outer iteration with dir_index 0
            // entry = 0x1002000, inner loop ptable_index 0 -> physical_addr = 0x0, 1 => physical_addr = 0x4096 and so on...
            entry[ptable_index] = physical_addr | flags;
        };
        // Update directory offset for the next iteration
        dir_offset += (PAGE_SIZE * PAGE_TABLE_ENTRIES);
        // Set the page directory entry to the page table's address with flags
        // after first iteration dir_index is 1 and dir_offset 4194304 or 0x400000
        directory[dir_index] = (PageEntry)entry | flags | PAGE_READ_WRITE;
    };
    // Allocate memory for the overall page directory structure
    PageDirectory *kpage_dir = kcalloc(sizeof(PageDirectory));
    kpage_dir->directory = directory;
    return kpage_dir;
};
