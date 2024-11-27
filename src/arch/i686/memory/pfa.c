/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"
#include "string.h"

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
	const size_t total_frames = BITMAP_SIZE * 32;
	size_t used = 0, free = 0;
	printf("\n\n====================================\n");
	printf("          PFA STATISTICS            \n");
	printf("====================================\n");
	printf("Total Frames: 			%d\n", total_frames);

	for (size_t i = 0; i < BITMAP_SIZE; i++) {
		const uint64_t value = self->frames_bitmap[i];
		const size_t first_bit = i * 32;
		const size_t last_bit = ((i + 1) * 32) - 1;
 
		if (verbose) {
			printf("Frames %d - %d: ", first_bit, last_bit);
		};

		for (int bit = 31; bit >= 0; bit--) {
			const bool is_used = value & (1 << bit);

			if (is_used) {
				used++;

				if (verbose) {
					printf("U");
				};
			} else {
				free++;

				if (verbose) {
					printf("F");
				};
			};
		};

		if (verbose) {
			printf("\n");
		};
	};
	printf("Used Frames:   		%f (%f%%)\n", (double)used, (100.0 * used) / total_frames);
	printf("Free Frames:   		%f (%f%%)\n", (double)free, (100.0 * free) / total_frames);
	printf("Memory In-Use: 		%f KiB\n", (double)(used * PAGE_SIZE) / 1024);
	printf("Free Memory:   		%f KiB\n", (double)(free * PAGE_SIZE) / 1024);
	printf("====================================\n");
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