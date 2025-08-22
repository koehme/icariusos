/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"
#include "hal.h"
#include "icarius.h"
#include "stdio.h"
#include "string.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t pfa_init(const pmm_boot_adapter_t* adapter, const u8* bitmap_addr);
pfa_t* pfa_get(void);
kresult_t pfa_mark(const u64 base, const u64 length, const pfa_state_t state);
kresult_t pfa_dump(const bool verbose);
uptr pfa_alloc(void);
kresult_t pfa_dealloc(const usize frame);
kresult_t pfa_dump_used_frames(void);

/* INTERNAL API */
static pfa_t _pfa;

kresult_t pfa_init(const pmm_boot_adapter_t* adapter, const u8* bitmap_addr)
{
	pfa_t* pfa = pfa_get();

	if (!adapter)
		return kresult_err(-K_EINVAL, "No pmm boot adapter provided");

	if (!bitmap_addr)
		return kresult_err(-K_EINVAL, "No bitmap addr provided");

	memcpy(&pfa->info, adapter, sizeof(pfa->info));
	pfa->bitmap_addr = (u8*)bitmap_addr;
	return kresult_ok(NULL);
};

pfa_t* pfa_get(void) { return &_pfa; };

kresult_t pfa_dump(const bool verbose)
{
	pfa_t* pfa = pfa_get();

	kprintf("----------------------------------------------------------\n");
	kprintf("\t\t[ Physical Memory Allocator ]\t\t\n");
	kprintf("----------------------------------------------------------\n");
	kprintf("\tTOTAL FRAMES\t\t%d\n", pfa->info.total_frames);
	kprintf("\tBITMAP SIZE\t\t%d\n", pfa->info.bitmap_size);
	kprintf("\tHHDM OFFSET\t\t%P\n", (void*)pfa->info.hhdm_offset);
	kprintf("\tPHY START\t\t%P\n", (void*)pfa->info.start_addr);
	kprintf("----------------------------------------------------------\n");

	usize used_frames = 0;

	for (usize frame = 0; frame < pfa->info.total_frames; frame++) {
		const usize byte_index = frame / 8u;
		const u8 bit_index = (u8)(frame % 8u);
		const u8 byte = pfa->bitmap_addr[byte_index];
		const u8 bit = (byte >> bit_index) & 1u;

		used_frames += bit;

		if (verbose) {
			kprintf("%d", bit);

			if ((frame + 1) % 64 == 0) {
				const usize start = (frame + 1) - 64;
				const usize end = frame;
				kprintf(" [ Start: %d -- End %d ]\n", start, end);
			};
		};
	};
	const usize free_frames = pfa->info.total_frames - used_frames;

	kprintf("\n");
	kprintf("----------------------------------------------------------\n");
	kprintf("\tUSED FRAMES\t\t%d\n", used_frames);
	kprintf("\tFREE FRAMES\t\t%d\n", free_frames);
	kprintf("----------------------------------------------------------\n");
	return kresult_ok(NULL);
};

kresult_t pfa_dump_used_frames(void)
{
	pfa_t* pfa = pfa_get();

	kprintf("\tUSED FRAMES\t\t\n");

	for (usize frame = 0; frame < pfa->info.total_frames; frame++) {
		const usize byte_index = frame / 8u;
		const u8 bit_index = (u8)(frame % 8u);
		const u8 byte = pfa->bitmap_addr[byte_index];
		const u8 mask = 1u << bit_index;

		if ((byte & mask) != FRAME_FREE) {
			const uptr phys_addr = frame * PAGE_SIZE;
			kprintf("Frame=%d Phys=%P Virt(HHDM)=%P\n", frame, (void*)phys_addr, (void*)(phys_addr + pfa->info.hhdm_offset));
		};
	};
	return kresult_ok(NULL);
};

kresult_t pfa_dealloc(const usize frame)
{
	pfa_t* pfa = pfa_get();

	const uptr phys_addr = frame * PAGE_SIZE;

	if (phys_addr % PAGE_SIZE != 0)
		return kresult_err(-K_EINVAL, "Physical address is not page aligned");

	if (frame >= pfa->info.total_frames)
		return kresult_err(-K_EINVAL, "Frame is out of pfa boundary");

	const usize byte_index = frame / 8u;
	const u8 bit_index = (u8)(frame % 8u);
	const u8 byte = pfa->bitmap_addr[byte_index];
	const u8 mask = 1u << bit_index;

	if ((byte & mask) == FRAME_FREE)
		return kresult_err(-K_EINVAL, "Frame is already freed");

	pfa->bitmap_addr[byte_index] &= ~mask;
	return kresult_ok(NULL);
};

uptr pfa_alloc(void)
{
	pfa_t* pfa = pfa_get();

	for (usize frame = 0; frame < pfa->info.total_frames; frame++) {
		const usize byte_index = frame / 8u;
		const u8 bit_index = (u8)(frame % 8u);
		const u8 byte = pfa->bitmap_addr[byte_index];
		const u8 mask = 1u << bit_index;

		if ((byte & mask) == FRAME_FREE) {
			pfa->bitmap_addr[byte_index] = byte | mask;
			const uptr phys_addr = frame * PAGE_SIZE;
			return phys_addr;
		};
	};
	return 0x0;
};

kresult_t pfa_mark(const u64 base, const u64 length, const pfa_state_t state)
{
	pfa_t* pfa = pfa_get();

	if (!length)
		return kresult_err(-K_EINVAL, "Range length is zero");

	const u64 start_al = ALIGN_DOWN(base, PAGE_SIZE);
	const u64 end_al = ALIGN_UP(base + length, PAGE_SIZE);

	const u64 first_frame = start_al / PAGE_SIZE;
	const u64 last_frame = end_al / PAGE_SIZE;

	if (first_frame > pfa->info.total_frames)
		return kresult_err(-K_EINVAL, "PFA exhausted");

	for (usize frame = first_frame; frame < last_frame; frame++) {
		const usize byte_index = frame / 8u;
		const u8 bit_index = (u8)(frame % 8u);
		const u8 mask = 1u << bit_index;

		switch (state) {
		case FRAME_FREE:
			pfa->bitmap_addr[byte_index] &= ~mask;
			break;
		case FRAME_USED:
			pfa->bitmap_addr[byte_index] |= mask;
		default:;
		};
	};
	return kresult_ok(NULL);
};