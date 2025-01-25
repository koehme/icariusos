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

#define PAGE_SIZE (1024 * 4 * 1024)
#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4
#define PAGE_PS 0x80 // Page Size (PS-Bit) = 4 MiB Pages

uint32_t* page_create_directory(uint32_t flags);
void page_set_directory(uint32_t* new_page_dir);
uint32_t* page_get_directory(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(uint32_t virt_addr);
uint32_t page_get_phys_addr(const uint32_t virt_addr);

#endif