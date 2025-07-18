#include <stdio.h>
#include <task.h>

void kidle(void)
{
	for (;;) {
		asm volatile("sti; hlt");
	};
	return;
};