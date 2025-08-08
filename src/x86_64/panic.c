/**
 * @file panic.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "panic.h"
#include "fb.h"
#include "kernel.h"

/* EXTERNAL API */

/* PUBLIC API */
noreturn void panic(void);

/* INTERNAL API */

noreturn void panic(void)
{
	fb_clear(RED);

	for (;;) {
		asm_halt();
	};
	__builtin_unreachable();
};