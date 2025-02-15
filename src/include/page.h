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

#include "icarius.h"
#include "pfa.h"
#include "string.h"
#include "task.h"

void page_dump_dir(uint32_t* dir);
uint32_t* page_create_dir(uint32_t flags);
void page_set_dir(uint32_t* self);
uint32_t* page_get_dir(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_dir(uint32_t* page_directory, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_between(uint32_t* dir, uint32_t virt_start, uint32_t virt_end, uint32_t flags);
void page_unmap_dir(uint32_t* dir, const uint32_t virt_addr);
void page_unmap_between(uint32_t* dir, uint32_t virt_start_addr, uint32_t virt_end_addr);
uint32_t page_get_phys_addr(uint32_t* dir, const uint32_t virt_addr);
void page_restore_kernel_dir(void);

#endif