/**
 * @file pfa.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PFA_H
#define PFA_H

#include <stdint.h>

#define MAX_P_MEMORY 0x100000000
#define PAGE_SIZE 4096
#define BITMAP_SIZE (MAX_P_MEMORY / PAGE_SIZE / 8)
#define FRAME_USED 0xFF
#define FRAME_FREE 0x00

typedef struct pfa_t {
	uint8_t frames_bitmap[BITMAP_SIZE];
} pfa_t;

void pfa_init(pfa_t* self);
void pfa_dump(const pfa_t* self);
void pfa_mark_free(pfa_t* self, uint64_t frame);

#endif