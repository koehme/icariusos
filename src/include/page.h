/**
 * @file page.h
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

#include "pfa.h"

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void unmap_page(uint32_t virt_addr);
uint32_t get_physical_address(uint32_t virt_addr);

#endif