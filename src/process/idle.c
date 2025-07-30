#include "idt.h"
#include "stdio.h"

void kidle(void)
{
	while (1) {
		/*
		asm_do_cli();
		printf("x");
		asm_do_sti();
		*/
		asm volatile("hlt");
	};
	return;
};