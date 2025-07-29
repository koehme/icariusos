#include "page.h"
#include <stdio.h>
#include <task.h>

void kidle(void)
{
	while (1) {
		asm_do_cli();
		printf("x");
		asm_do_sti();
		asm volatile("sti; hlt");
	};
	return;
};