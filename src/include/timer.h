/**
 * @file timer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "icarius.h"

typedef struct timer {
	uint64_t ticks; // Number of ticks since system booted
	uint32_t hz;	// Frequency (in Hertz) at which the timer_t operates
} timer_t;

void timer_init(timer_t* self, const uint32_t hz);

#endif