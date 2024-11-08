#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

typedef uint32_t page_t;
typedef uint32_t* page_table_t;

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

void page_paging_init();
void page_map_virt_to_phys(void* virt_addr, void* phys_addr, const uint32_t flags);
void page_umap(void* virt_addr);
void* page_get_phys_addr(void* virt_addr);

#endif