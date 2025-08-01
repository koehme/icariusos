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
		/*
		asm_do_cli();
		kprintf("x");
		asm_do_sti();
		*/
		asm volatile("hlt");
	};
	return;
};