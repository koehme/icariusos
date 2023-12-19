/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page.h"
#include "icarius.h"
#include <stdbool.h>

extern void asm_page_load(PageEntry *self);

PageDirectory kpage_dir = {
    .directory = 0x0,
};

static PageEntry *ptr_kpage_curr_dir = 0x0;

static bool is_addr_aligned(void *addr)
{
    const bool is_addr_aligned = ((PageEntry)addr % PAGE_SIZE) == 0;
    return is_addr_aligned;
};

/**
 * @brief Allocates memory for the page directory and page tables and initializes them.
 * @param flags The flags to be used for initialization.
 * @return A pointer to the initialized page directory structure.
 */
void page_init_directory(PageDirectory *self, const uint8_t flags)
{
    // Allocate memory for the 4kb page directory
    PageEntry *pd = kcalloc(sizeof(PageEntry) * PAGE_TABLE_ENTRIES);
    size_t pd_offset = 0;
    // Loop through each entry in the page directory
    for (size_t pd_index = 0; pd_index < PAGE_TABLE_ENTRIES; pd_index++)
    {
        // Allocate memory for a page table
        PageEntry *pt_entry = kcalloc(sizeof(PageEntry) * PAGE_TABLE_ENTRIES);
        // Loop through each entry in the page table
        for (size_t pt_index = 0; pt_index < PAGE_TABLE_ENTRIES; pt_index++)
        {
            const uint32_t phy_addr = pd_offset + (pt_index * PAGE_SIZE);
            // Set the page table entry to the physical address with flags
            // for example in the first outer iteration with pd_index 0
            // entry = 0x1002000, inner loop pt_index 0 -> phy_addr = 0x0, 1 => phy_addr = 0x4096 and so on...
            pt_entry[pt_index] = phy_addr | flags;
        };
        // Update directory offset for the next iteration
        pd_offset += (PAGE_SIZE * PAGE_TABLE_ENTRIES);
        // Set the page directory entry to the page table's address with flags
        // after first iteration pd_index is 1 and pd_offset 4194304 or 0x400000
        pd[pd_index] = (PageEntry)pt_entry | flags;
    };
    kpage_dir.directory = pd;
    return;
};

void page_switch(PageEntry *dir)
{
    asm_page_load(dir);
    ptr_kpage_curr_dir = dir;
    return;
};

/**
 * @brief Calculates the Page Directory and Page Table indexes for a given virtual address.
 * Takes a virtual address and computes the corresponding indexes in the Page Directory (PD) and Page Table (PT).
 * @param virt_addr The virtual address for which to calculate indexes.
 * @return A structure containing the Page Directory and Page Table indexes.
 *
 * Assumes a 32-bit paging system with 4 KB page size.
 *
 * Virtual Address Format
 *
 * | Directory Index  | Table Index | Offset  |
 * |------------------|-------------|-------- |
 * |      10 bits     |   10 bits   |  12 bits|
 */
PageVirtual virt_address_get_pd_indicies(void *virt_addr)
{
    const bool is_aligned = is_addr_aligned(virt_addr);

    if (!is_aligned)
    {
        kpanic("Virtual memory address must be aligned!");
    };
    // Extracts the 10 most significant bits, which represent the index into the Page Directory
    const size_t pd_index = ((uint32_t)virt_addr >> 22);
    // Extracts the middle 10 bits, which represent the index into the Page Table
    const size_t pt_index = ((uint32_t)virt_addr >> 12) & PAGE_MASK;

    const PageVirtual pv = {
        .pd_index = pd_index,
        .pt_index = pt_index,
    };
    return pv;
};

/**
 * @brief Maps a virtual address to a physical address in the page tables.
 * @param dir       Pointer to the Page Directory.
 * @param virt_addr Virtual address to be mapped.
 * @param phy_addr  Physical address to be mapped to the virtual address.
 *                  (Includes flags such as PAGE_PRESENT, PAGE_READ_WRITE, etc.)
 * @return 0 on success, -1 on failure.
 */
int virt_address_map(PageEntry *dir, void *virt_addr, const uint32_t phy_addr)
{
    const bool is_aligned = is_addr_aligned(virt_addr);

    if (!is_aligned)
    {
        return -1;
    };
    const PageVirtual pv = virt_address_get_pd_indicies(virt_addr);
    const uint32_t pd_entry = dir[pv.pd_index];
    uint32_t *pt_entry = (uint32_t *)(pd_entry & 0b11111111111111111111000000000000);
    pt_entry[pv.pt_index] = phy_addr;
    return 0;
};