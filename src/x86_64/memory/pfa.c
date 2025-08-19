/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t pfa_init(const pmm_boot_adapter_t* info, const uint8_t* bitmap_addr);
pfa_t* pfa_get(void);
kresult_t pfa_mark(const uint64_t base, const uint64_t length, const pfa_state_t state);
kresult_t pfa_dump(const bool verbose);
uintptr_t pfa_alloc(void);
kresult_t pfa_dealloc(const uint64_t frame);
kresult_t pfa_dump_used_frames(void);

/* INTERNAL API */
static pfa_t _pfa;

kresult_t pfa_init(const pmm_boot_adapter_t* info, const uint8_t* bitmap_addr)
{
	pfa_t* pfa = pfa_get();

	if (!pfa)
		return kresult_err(-K_ENODEV, "No pfa available");

	if (!info)
		return kresult_err(-K_EINVAL, "No pmm boot adapter provided");

	if (!bitmap_addr)
		return kresult_err(-K_EINVAL, "No bitmap_addr provided");

	memcpy(&pfa->info, info, sizeof(pfa->info));
	pfa->bitmap_addr = (uint8_t*)bitmap_addr;
	return kresult_ok(NULL);
};

pfa_t* pfa_get(void) { return &_pfa; };

kresult_t pfa_dump(const bool verbose)
{
	pfa_t* pfa = pfa_get();

	if (!pfa)
		return kresult_err(-K_ENODEV, "No pfa available");

	kprintf("----------------------------------------------------------\n");
	kprintf("\t\t[ Physical Memory Allocator ]\t\t\n");
	kprintf("----------------------------------------------------------\n");
	kprintf("\tTOTAL FRAMES\t\t%d\n", pfa->info.total_frames);
	kprintf("\tBITMAP SIZE\t\t%d\n", pfa->info.bitmap_size);
	kprintf("\tHHDM OFFSET\t\t%p\n", pfa->info.hhdm_offset);
	kprintf("\tPHY START\t\t%p\n", pfa->info.start_addr);
	kprintf("----------------------------------------------------------\n");

	size_t used_frames = 0;

	for (size_t frame = 0; frame < pfa->info.total_frames; frame++) {
		const size_t byte_index = frame / 8u;
		const uint8_t bit_index = (uint8_t)(frame % 8u);
		const uint8_t byte = pfa->bitmap_addr[byte_index];
		const uint8_t bit = (byte >> bit_index) & 1u;

		used_frames += bit;

		if (verbose) {
			kprintf("%d", bit);

			if ((frame + 1) % 64 == 0) {
				const size_t start = (frame + 1) - 64;
				const size_t end = frame;
				kprintf(" [ Start: %d -- End %d ]\n", start, end);
			};
		};
	};
	const size_t free_frames = pfa->info.total_frames - used_frames;

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

	if (!pfa)
		return kresult_err(-K_ENODEV, "No pfa available");

	kprintf("\tUSED FRAMES\t\t\n");

	for (size_t frame = 0; frame < pfa->info.total_frames; frame++) {
		const size_t byte_index = frame / 8u;
		const uint8_t bit_index = (uint8_t)(frame % 8u);
		const uint8_t byte = pfa->bitmap_addr[byte_index];
		const uint8_t mask = 1u << bit_index;

		if ((byte & mask) != FRAME_FREE) {
			const uintptr_t phys_addr = frame * PAGE_SIZE;
			kprintf("Frame=%d Phys=%p Virt(HHDM)=%p\n", frame, phys_addr, (phys_addr + pfa->info.hhdm_offset));
		};
	};
	return kresult_ok(NULL);
};

kresult_t pfa_dealloc(const size_t frame)
{
	pfa_t* pfa = pfa_get();

	if (!pfa)
		return kresult_err(-K_ENODEV, "No pfa available");

	const uintptr_t phys_addr = frame * PAGE_SIZE;

	if (phys_addr % PAGE_SIZE != 0)
		return kresult_err(-K_EINVAL, "Physical address is not page aligned");

	if (frame >= pfa->info.total_frames)
		return kresult_err(-K_EINVAL, "Frame is out of pfa boundary");

	const size_t byte_index = frame / 8u;
	const uint8_t bit_index = (uint8_t)(frame % 8u);
	const uint8_t byte = pfa->bitmap_addr[byte_index];
	const uint8_t mask = 1u << bit_index;

	if ((byte & mask) == FRAME_FREE)
		return kresult_err(-K_EINVAL, "Frame is already freed");

	pfa->bitmap_addr[byte_index] &= ~mask;
	return kresult_ok(NULL);
};

uintptr_t pfa_alloc(void)
{
	pfa_t* pfa = pfa_get();

	if (!pfa)
		return 0x0;

	for (size_t frame = 0; frame < pfa->info.total_frames; frame++) {
		const size_t byte_index = frame / 8u;
		const uint8_t bit_index = (uint8_t)(frame % 8u);
		const uint8_t byte = pfa->bitmap_addr[byte_index];
		const uint8_t mask = 1u << bit_index;

		if ((byte & mask) == FRAME_FREE) {
			pfa->bitmap_addr[byte_index] = byte | mask;
			const uintptr_t phys_addr = frame * PAGE_SIZE;
			return phys_addr;
		};
	};
	return 0x0;
};

kresult_t pfa_mark(const uint64_t base, const uint64_t length, const pfa_state_t state)
{
	pfa_t* pfa = pfa_get();

	if (!pfa)
		return kresult_err(-K_ENODEV, "No pfa available");

	if (!length)
		return kresult_err(-K_EINVAL, "Range length is zero");

	const uint64_t start_al = ALIGN_DOWN(base, PAGE_SIZE);
	const uint64_t end_al = ALIGN_UP(base + length, PAGE_SIZE);

	const uint64_t first_frame = start_al / PAGE_SIZE;
	const uint64_t last_frame = end_al / PAGE_SIZE;

	if (first_frame > pfa->info.total_frames)
		return kresult_err(-K_EINVAL, "PFA exhausted");

	for (size_t frame = first_frame; frame < last_frame; frame++) {
		const size_t byte_index = frame / 8u;
		const uint8_t bit_index = (uint8_t)(frame % 8u);
		const uint8_t mask = 1u << bit_index;

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