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
#include "string.h"
#include "task.h"

#define PAGE_SIZE (1024 * 4 * 1024)
// Flags
#define PAGE_PRESENT 0x1   // Page is present in physical memory
#define PAGE_WRITABLE 0x2  // Page is writable (read/write access)
#define PAGE_USER 0x4	   // User-mode access allowed (0 = kernel-only)
#define PAGE_PWT 0x8	   // Page Write-Through enabled
#define PAGE_PCD 0x10	   // Page Cache Disable (no caching for this page)
#define PAGE_ACCESSED 0x20 // Set by the CPU when the page is accessed
#define PAGE_DIRTY 0x40	   // Set by the CPU when the page is written to
#define PAGE_PS 0x80	   // Page Size (1 = 4 MiB page, only in PDE)
#define PAGE_GLOBAL 0x100  // Global page (remains cached in TLB across context switches)

void page_dump_dir(uint32_t* dir);
uint32_t* page_create_dir(uint32_t flags, void (*user_eip)());
void page_set_dir(uint32_t* self);
uint32_t* page_get_dir(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_dir(uint32_t* page_directory, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_between(uint32_t* dir, uint32_t virt_start, uint32_t virt_end, uint32_t flags);
void page_unmap(uint32_t virt_addr);
uint32_t page_get_phys_addr(uint32_t* dir, const uint32_t virt_addr);
void page_restore_kernel_dir(void);

#endif