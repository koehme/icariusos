/**
 * @file umode.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdint.h>

#include "kernel.h"
#include "umode.h"

uintptr_t umode_create_user_stack(void)
{
	uintptr_t frame = pfa_alloc();

	if (!frame) {
		panic("[Error] Out of Memory, while creating a user stack");
	};
	// Map physical frame into ring 3
	// TODO
	return 0x0;
};