/**
 * @file pfa.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "hal.h"
#include "icarius.h"
#include "kres.h"
#include "stdio.h"
#include "string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct pmm_boot_adapter {
	uintptr_t hhdm_offset; // Directly access physical frames in the upper half 0xffff800000000000
	uintptr_t start_addr;  // Physical start address of the PMM bitmap itself
	uint64_t bitmap_size;  // Size of the PMM bitmap in aligned bytes
	uint64_t total_frames; // Total number of physical frames managed by the PMM
} pmm_boot_adapter_t;

typedef enum pfa_state {
	FRAME_FREE = 0x0,
	FRAME_USED = 0x1,
} pfa_state_t;

typedef struct pfa {
	pmm_boot_adapter_t info;
	uint8_t* bitmap_addr; // Virtual start address of the PMM bitmap itself
} pfa_t;

kresult_t pfa_init(const pmm_boot_adapter_t* info, const uint8_t* bitmap_addr);
pfa_t* pfa_get(void);
kresult_t pfa_mark(const uint64_t base, const uint64_t length, const pfa_state_t state);
kresult_t pfa_dump(const bool verbose);
uintptr_t pfa_alloc(void);
kresult_t pfa_dealloc(const size_t frame);
kresult_t pfa_dump_used_frames(void);