/**
 * @file pfa.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "kres.h"
#include "types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct pmm_boot_adapter {
	uptr hhdm_offset; // Directly access physical frames in the upper half 0xffff800000000000
	uptr start_addr;  // Physical start address of the PMM bitmap itself
	u64 bitmap_size;  // Size of the PMM bitmap in aligned bytes
	u64 total_frames; // Total number of physical frames managed by the PMM
} pmm_boot_adapter_t;

typedef enum pfa_state {
	FRAME_FREE = 0x0,
	FRAME_USED = 0x1,
} pfa_state_t;

typedef struct pfa {
	pmm_boot_adapter_t info;
	u8* bitmap_addr; // Virtual start address of the PMM bitmap itself
} pfa_t;

kresult_t pfa_init(const pmm_boot_adapter_t* adapter, const u8* bitmap_addr);
pfa_t* pfa_get(void);
kresult_t pfa_mark(const u64 base, const u64 length, const pfa_state_t state);
kresult_t pfa_dump(const bool verbose);
uptr pfa_alloc(void);
kresult_t pfa_dealloc(const usize frame);
kresult_t pfa_dump_used_frames(void);