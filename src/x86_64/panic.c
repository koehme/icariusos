/**
 * @file panic.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "panic.h"

/* EXTERNAL API */
extern void asm_hlt(void);

/* PUBLIC API */
noreturn void panic(void);

/* INTERNAL API */
// -

noreturn void panic(void)
{
	for (;;) {
		asm_hlt();
	};
	__builtin_unreachable();
};