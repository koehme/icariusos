/**
 * @file pfa.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PFA_H
#define PFA_H

#include <stdbool.h>
#include <stdint.h>

#include "icarius.h"
#include "page.h"

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