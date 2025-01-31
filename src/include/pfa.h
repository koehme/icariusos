/**
 * @file pfa.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PFA_H
#define PFA_H

#include <stdbool.h>
#include <stdint.h>

#include "page.h"

#define MAX_PHYSICAL_MEMORY 0x100000000
#define PAGE_SIZE (1024 * 4 * 1024)
#define MAX_FRAMES (MAX_PHYSICAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE (MAX_FRAMES / 32)

#define KERNEL_PHYS_BASE 0x00000000
#define KERNEL_PHYS_END 0x02FFFFFF

#define KERNEL_VIRT_BASE 0xC0000000

#define FRAMEBUFFER_VIRT_BASE 0xE0000000
#define FRAMEBUFFER_PHYS_BASE 0xFD000000
#define FRAMEBUFFER_WIDTH 800
#define FRAMEBUFFER_HEIGHT 600
#define FRAMEBUFFER_DEPTH (32 / 8)
#define FRAMEBUFFER_SIZE ((FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT) * FRAMEBUFFER_DEPTH)

typedef struct pfa {
	uint32_t frames_bitmap[BITMAP_SIZE];
} pfa_t;

void pfa_init(pfa_t* self);
void pfa_dump(const pfa_t* self, const bool verbose);
void pfa_set(pfa_t* self, uint64_t frame);
void pfa_clear(pfa_t* self, uint64_t frame);
bool pfa_test(const pfa_t* self, const uint64_t frame);
uint64_t pfa_alloc(void);

#endif