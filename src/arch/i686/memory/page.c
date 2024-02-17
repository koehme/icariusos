/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "kernel.h"
#include <stdbool.h>

extern void asm_page_load(uint32_t *self);

PageDirectory kpage_dir = {
    .directory = 0x0,
};

static uint32_t *ptr_kpage_curr_dir = 0x0;

// Check if the address is aligned to the page size
static bool is_addr_aligned(void *addr)
{
    const bool is_addr_aligned = ((uint32_t)addr % 4096) == 0;
    return is_addr_aligned;
};

// Initialize the page directory with the specified flags
void page_init_directory(PageDirectory *self, const uint8_t flags)
{
    // Allocate memory for the 4kb page directory
    uint32_t *pd = kcalloc(sizeof(uint32_t) * 1024);
    size_t pd_offset = 0;
    // Loop through each entry in the page directory
    for (size_t pd_index = 0; pd_index < 1024; pd_index++)
    {
        // Allocate memory for a page table
        uint32_t *pt_entry = kcalloc(sizeof(uint32_t) * 1024);
        // Loop through each entry in the page table
        for (size_t pt_index = 0; pt_index < 1024; pt_index++)
        {
            // Calculate the physical frame address
            const uint32_t frame = pd_offset + (pt_index * 4096);
            // Set the page table entry to the physical frame address with flags
            // for example in the first outer iteration with pd_index 0
            // pt_entry = 0x1002000, inner loop pt_index 0 -> frame = 0x0, 1 => frame = 0x4096 and so on...
            pt_entry[pt_index] = frame | flags;
        };
        // Update directory offset for the next iteration
        pd_offset += (4096 * 1024);
        // Set the page directory entry to the page table's address with flags
        // after first iteration pd_index is 1 and pd_offset 4194304 or 0x400000
        pd[pd_index] = (uint32_t)pt_entry | flags;
    };
    // Set the global page directory to the allocated page directory
    kpage_dir.directory = pd;
    return;
};

// Switches the current page directory to the one specified
void page_switch(uint32_t *dir)
{
    // Load the given page directory into the CR3 register to switch to the new memory mapping
    asm_page_load(dir);
    // Update the pointer to the current page directory
    ptr_kpage_curr_dir = dir;
    return;
};

// Extracts page directory and table indices from a virtual memory address
static VMemTranslation translate_vaddress(void *vaddr)
{
    const bool is_aligned = is_addr_aligned(vaddr);

    if (!is_aligned)
    {
        kpanic("PageError: Virtual memory address must be aligned!");
    };
    // * | Directory Index   | Table Index | Offset   |
    // * |------------------ |-------------|----------|
    // * |      10 Bits      |   10 Bits   |  12 Bits |
    // Extracts the 10 most significant bits, which represent the index into the Page Directory
    const uint32_t pd_index = ((uint32_t)vaddr >> 22);
    // Extracts the middle 10 bits, which represent the index into the Page Table
    const uint32_t pt_index = ((uint32_t)vaddr >> 12) & PAGE_TABLE_INDEX;

    const VMemTranslation translation = {
        .pd_index = pd_index,
        .pt_index = pt_index,
    };
    return translation;
};

int8_t map_frame_to_address(uint32_t *dir, void *vaddr, const uint32_t frame)
{
    const bool is_aligned = is_addr_aligned(vaddr);

    if (!is_aligned)
    {
        kprintf("PageError: Virtual memory address must be aligned!\n");
        return -1;
    };
    // Translate the virtual address to page indices
    const VMemTranslation translation = translate_vaddress(vaddr);
    // Access the page directory entry corresponding to the virtual address
    const uint32_t pd_entry = dir[translation.pd_index];
    // Extract the page table entry for the virtual address
    uint32_t *pt_entry = (uint32_t *)(pd_entry & 0b11111111111111111111000000000000);
    // Map the provided physical frame to the page table entry
    pt_entry[translation.pt_index] = frame;
    return 0;
};