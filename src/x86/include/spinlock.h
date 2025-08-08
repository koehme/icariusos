/**
 * @file spinlock.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef volatile uint32_t spinlock_t;

void spinlock_acquire(spinlock_t* self);
void spinlock_release(spinlock_t* self);

#endif