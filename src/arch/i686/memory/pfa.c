/**
 * @file pfa.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pfa.h"
#include "string.h"

pfa_t pfa = {};

void pfa_init(pfa_t* self)
{
	memset(self->frames_bitmap, FRAME_USED, BITMAP_SIZE);
	return;
};

void pfa_mark_free(pfa_t* self, uint64_t frame) { return; };

void pfa_dump(const pfa_t* self)
{
	const size_t total_frames = BITMAP_SIZE * 8;
	size_t used = 0;
	size_t free = 0;

	for (size_t i = 0; i < total_frames; i++) {
		const size_t byte_index = i / 8;
		const size_t bit_offset = i % 8;
		const uint8_t bit = (self->frames_bitmap[byte_index] >> bit_offset) & 1;

		switch (bit) {
		case FRAME_USED:
			used++;
			break;
		case FRAME_FREE:
			free++;
			break;
		default:
			break;
		};
		// Print frames in blocks of 32 for better visualization
		if (i % 32 == 0) {
			printf("\nFrames %d - %d: ", (int)i, (int)(i + 31));
		};
		printf("%c", (bit == FRAME_USED) ? 'U' : 'F'); // 'U' for Used, 'F' for Free
	};

	printf("\n\n====================================\n");
	printf("          PFA STATISTICS            \n");
	printf("====================================\n");
	printf("Total Frames: %d\n", (int)total_frames);
	printf("Used Frames:   %d\n", (int)used);
	printf("Free Frames:   %d\n", (int)free);
	printf("Memory In-Use: %d KiB\n", (int)((used * PAGE_SIZE) / 1024));
	printf("Free Memory:   %d KiB\n", (int)((free * PAGE_SIZE) / 1024));
	printf("====================================\n");
	return;
};