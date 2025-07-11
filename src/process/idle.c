#include "idle.h"
#include "task.h"

void idle(void)
{
	for (;;) {
		asm volatile("hlt");
	};
	return;
};