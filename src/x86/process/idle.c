/**
 * @file idle.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "idt.h"
#include "stdio.h"

void kidle(void)
{
	for (;;) {
		asm volatile("hlt");
	};
	return;
};