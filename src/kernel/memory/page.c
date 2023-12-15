/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "icarius.h"

PageDirectory kpage_directory;

/**
 * @brief Initializes a page directory in the given PageDirectory structure.
 * @param self      Pointer to the PageDirectory structure to be initialized.
 * @param flags     Flags to be applied to each page table entry.
 * @param page_size Size of each page in the page table, typically 4096 for i686.
 * @param entries   Number of entries in the page directory, typically 1024 for i686.
 */
void page_init_directory(PageDirectory *self, const uint8_t flags, const int page_size, const int entries)
{
    // Allocate memory for 1024 page directory entries
    uint32_t *dir = kcalloc(sizeof(uint32_t) * entries);
    //                                                                                                        1024 * 4096       offset+=1024*4096
    // Offset to track the memory location for each page table in the first iteration offset is 0x0 in second 0x400000 in third 0x800000
    size_t offset = 0;
    // Loop through each page directory entry
    for (int directory_index = 0; directory_index < entries; directory_index++)
    {
        // Allocate memory for 1024 page table entries
        uint32_t *page_table = kcalloc(sizeof(uint32_t) * entries);
        // Loop through each page table entry
        for (int table_index = 0; table_index < entries; table_index++)
        {
            // Set the page table entry with the calculated memory location and flags
            // example: table_index = 0, 1, 2 ...
            // 0 + ( 0 * 4096 ) = 0, 0 + ( 1 * 4096 ) = 4096, 0 + ( 2 * 4096 ) = 8192 and so on ...
            page_table[table_index] = (offset + (table_index * page_size)) | flags;
        };
        // Update the offset to point to the next memory location
        offset += entries * page_size;
        // Set the page directory entry with the page table's base address and flags
        dir[directory_index] = (uint32_t)page_table | flags | PAGE_WRITEABLE;
    };
    self->directory = dir;
    return;
};
