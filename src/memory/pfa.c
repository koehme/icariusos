/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"
#include "string.h"

/* PUBLIC API */
void pfa_init(pfa_t* self);
void pfa_set(pfa_t* self, const uint64_t frame);
void pfa_clear(pfa_t* self, const uint64_t frame);
bool pfa_test(const pfa_t* self, const uint64_t frame);
void pfa_dump(const pfa_t* self, const bool verbose);
uint64_t pfa_alloc(void);

/* INTERNAL API */
static inline uint32_t _index_from_bit(const uint64_t frame);
static inline uint32_t _offset_from_bit(const uint64_t frame);

pfa_t pfa = {
    .frames_bitmap =
	{
	    0,
	},
};

static inline uint32_t _index_from_bit(const uint64_t frame) { return frame / 32; };

static inline uint32_t _offset_from_bit(const uint64_t frame) { return frame % 32; };

void pfa_init(pfa_t* self)
{
	for (size_t i = 0; i < (MAX_FRAMES / 32); i++) {
		self->frames_bitmap[i] = 0xFFFFFFFF;
	};
	return;
};

void pfa_set(pfa_t* self, const uint64_t frame)
{
	const uint64_t index = frame / 32;
	const uint64_t offset = frame % 32;
	self->frames_bitmap[index] |= (1 << offset);
	return;
};

void pfa_clear(pfa_t* self, const uint64_t frame)
{
	const uint64_t index = frame / 32;
	const uint64_t offset = frame % 32;
	self->frames_bitmap[index] &= ~(1 << offset);
	return;
};

bool pfa_test(const pfa_t* self, const uint64_t frame)
{
	const uint64_t index = _index_from_bit(frame);
	const uint64_t offset = _offset_from_bit(frame);
	return (self->frames_bitmap[index] & (1 << offset)) != 0;
};

void pfa_dump(const pfa_t* self, const bool verbose)
{
	if (!self) {
		kprintf("Invalid PFA Pointer!\n");
		return;
	};
	const size_t frames_per_block = sizeof(self->frames_bitmap[0]) * 8;
	const size_t total_frames = BITMAP_SIZE * frames_per_block;
	size_t used = 0;
	size_t free = 0;

	kprintf("\n");
	kprintf("====================================\n");
	kprintf("          PFA STATISTICS            \n");
	kprintf("====================================\n");
	kprintf("Total Frames: 			%d\n", total_frames);

	for (size_t block_index = 0; block_index < BITMAP_SIZE; block_index++) {
		const uint64_t frame_block = self->frames_bitmap[block_index];
		const size_t frame_start_index = block_index * frames_per_block;
		const size_t frame_end_index = frame_start_index + frames_per_block - 1;

		if (verbose) {
			kprintf("Frames %d - %d: ", frame_start_index, frame_end_index);
		};

		for (size_t frame = 0; frame < frames_per_block; frame++) {
			const bool is_used = frame_block & (1 << frame);
			is_used ? used++ : free++;

			if (verbose) {
				kprintf(is_used ? "U" : "F");
			};
		};

		if (verbose) {
			kprintf("\n");
		};
	};
	kprintf("Used Frames:   		%f (%f%%)\n", (double)used, (100.0 * used) / total_frames);
	kprintf("Free Frames:   		%f (%f%%)\n", (double)free, (100.0 * free) / total_frames);
	kprintf("Memory In-Use: 		%f KiB\n", (double)(used * PAGE_SIZE) / 1024);
	kprintf("Free Memory:   		%f KiB\n", (double)(free * PAGE_SIZE) / 1024);
	kprintf("====================================\n");
	return;
};

uint64_t pfa_alloc(void)
{
	for (uint32_t frame = 0; frame < MAX_FRAMES; frame++) {
		if (!pfa_test(&pfa, frame)) {
			pfa_set(&pfa, frame);
			const uint64_t phys_addr = frame * PAGE_SIZE;
			return phys_addr;
		};
	};
	return 0x0;
};