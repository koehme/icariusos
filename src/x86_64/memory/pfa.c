/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"
#include "kres.h"
#include "stdio.h"
#include "string.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
void pfa_init(const pmm_boot_adapter_t* info, const uint8_t* bitmap_addr);
pfa_t* pfa_get(void);
kresult_t pfa_mark(const uint64_t base, const uint64_t length, const pfa_state_t state);
kresult_t pfa_dump(void);

/* INTERNAL API */
static void _mark_free(pfa_t* pfa, const uint64_t frame);
static void _mark_used(pfa_t* pfa, const uint64_t frame);
static pfa_t _pfa;

void pfa_init(const pmm_boot_adapter_t* info, const uint8_t* bitmap_addr)
{
	memcpy(&_pfa.info, info, sizeof(_pfa.info));
	_pfa.bitmap_addr = (uint8_t*)bitmap_addr;
	return;
};

pfa_t* pfa_get(void) { return &_pfa; };

kresult_t pfa_dump(void)
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
	return kresult_ok(NULL);
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

	for (size_t curr_frame = first_frame; curr_frame < last_frame; curr_frame++)
		state == FRAME_FREE ? _mark_free(pfa, curr_frame) : _mark_used(pfa, curr_frame);
	return kresult_ok(NULL);
};

static void _mark_free(pfa_t* pfa, const uint64_t frame)
{
	const size_t which_byte = (size_t)(frame / 8u);
	const uint8_t bit_offset = (uint8_t)(frame % 8u);
	const uint8_t mask = (uint8_t)(1u << bit_offset);
	pfa->bitmap_addr[which_byte] &= ~mask;
	return;
};

static void _mark_used(pfa_t* pfa, const uint64_t frame)
{
	const size_t which_byte = (size_t)(frame / 8u);
	const uint8_t bit_offset = (uint8_t)(frame % 8u);
	const uint8_t mask = (uint8_t)(1u << bit_offset);
	pfa->bitmap_addr[which_byte] |= mask;
	return;
};
