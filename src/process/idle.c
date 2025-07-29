#include "page.h"
#include <stdio.h>
#include <task.h>

void kidle(void)
{
	while (1) {
		printf("x");
		asm volatile("sti; hlt");
	};
	return;
};