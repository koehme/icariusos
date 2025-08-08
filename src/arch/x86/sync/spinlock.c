/**
 * @file spinlock.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "spinlock.h"


extern uint32_t asm_xchg(volatile uint32_t* addr, const uint32_t new_val);

void spinlock_acquire(spinlock_t* self)
{
	while (asm_xchg(self, 1)) {
		;
	};
	return;
};

void spinlock_release(spinlock_t* self)
{
	*self = 0;
	return;
};