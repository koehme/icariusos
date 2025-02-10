/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"

void syscall_dispatch(int32_t syscall_id, interrupt_frame_t* frame)
{
	printf("=====================================\n");
	printf("  Syscall Invoked! (ID: %d)\n", syscall_id);
	printf("=====================================\n");

	// idt_dump_interrupt_frame(frame);
	page_restore_kernel_dir();
	asm_restore_kernel_segment();

	switch (syscall_id) {
	case 1: {
		printf("[INFO] Usermode Task EXITED! Back to Kernel-Land (Ring 0)\n");
		kernel_shell();
		break;
	};
	default: {
		printf("[INFO] Unknown Syscall: %d\n", syscall_id);
		break;
	};
	};
	return;
};