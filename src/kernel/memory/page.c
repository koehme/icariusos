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
    /*

        VISUALIZATION

        s_address   => 0x01000000 (real mem)

                        => 1024 Page Directory entries
                        [
                            0x0   ----------------  [   Page Table 0
                                                            .
                                                            .
                                                            .
                                                            1024 entries => each point to physical mem by the MMU in the CPU
                                                    ]

                            0x1000----------------  [   Page Table 1
                                                            .
                                                            .
                                                            .
                                                            1024 entries => each point to physical mem by the MMU in the CPU
                                                    ]

                        ]


    */
    // Create the pointer to the page directory himself
    // the frst index points to 0x0
    // the second indes points to 0x40000   => previous page holds 1024 blocks * 4096 byte = 0x4000000
    uint32_t *page_directory_entries = kcalloc(sizeof(uint32_t) * 1024);
    size_t offset = 0;

    // Create 1024 page tables
    for (int directory_index = 0; directory_index < 1024; directory_index++)
    {
        uint32_t *page_table_entries = kcalloc(sizeof(uint32_t) * 1024);

        // Initialize all 1024 pointers of the page directory
        for (int table_index = 0; table_index < 1024; table_index++)
        {
            page_table_entries[table_index] = (offset + (table_index * page_size)) | flags;
        };
        // Update the offset for the next iteration
        offset += 1024 * page_size;
        page_directory_entries[directory_index] = (uint32_t)page_table_entries | flags | PAGE_WRITEABLE;
    }

    // The structure is on the stack, but the page_directory_entries 32-bit pointer is aligned by our kernel heap
    self->directory = page_directory_entries;
    return;
};
