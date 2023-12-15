/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "icarius.h"

PageDirectory p_directory;

void page_init_directory(PageDirectory *self, const uint8_t flags, const uint16_t page_size)
{
    uint32_t *page_directory_entries = kcalloc(sizeof(uint32_t) * 1024);
    size_t offset = 0;

    for (int directory_index = 0; directory_index < 1024; directory_index++)
    {
        uint32_t *page_table_entries = kcalloc(sizeof(uint32_t) * 1024);

        for (int table_index = 0; table_index < 1024; table_index++)
        {
            page_table_entries[table_index] = (offset + (table_index * page_size)) | flags;
        };
        offset += 1024 * page_size;
        page_directory_entries[directory_index] = (uint32_t)page_table_entries | flags | PAGE_WRITEABLE;
    };
    self->directory = page_directory_entries;
    return;
};
