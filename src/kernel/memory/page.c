/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "icarius.h"

/**
 * @brief Initializes a page directory in the given PageDirectory structure.
 * @param self      Pointer to the PageDirectory structure to be initialized.
 * @param flags     Flags to be applied to each page table entry.
 * @param page_size Size of each page in the page table, typically 4096 for i686.
 * @param entries   Number of entries in the page directory, typically 1024 for i686.
 */
PageDirectory *page_init_directory(PageDirectory *self, const uint8_t flags, const int page_size, const int entries)
{
    uint32_t *directory = kcalloc(sizeof(uint32_t) * entries);
    self->directory = directory;
    return self;
};
