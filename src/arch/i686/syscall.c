/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"

typedef void (*syscall_handler_t)(int32_t);

typedef struct syscall_entry {
	int32_t num;
	syscall_handler_t handler;
} syscall_entry_t;

syscall_entry_t syscalls[MAX_SYSCALL] = {
    {
	0x0,
	0x0,
    },
};

static void _sys_exit(const int32_t code)
{
	printf("[INFO] Usermode Task EXITED with %d! Back to Kernel-Land (Ring 0)\n", code);
	kernel_shell();
	return;
};

static void _run_syscall(const int32_t syscall_id, const int32_t arg)
{
	if (syscall_id < 0 || syscall_id >= MAX_SYSCALL) {
		printf("[ERROR] Invalid Syscall Number: %d\n", syscall_id);
		return;
	};
	const syscall_entry_t syscall = syscalls[syscall_id];

	if (!syscall.handler) {
		printf("[ERROR] Unimplemented Syscall: %d\n", syscall_id);
		return;
	};
	syscall.handler(arg);
	return;
};

void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame)
{
	printf("=====================================\n");
	printf("  Syscall Invoked! (ID: %d)\n", syscall_id);
	printf("=====================================\n");

	// idt_dump_interrupt_frame(frame);
	page_restore_kernel_dir();
	asm_restore_kernel_segment();

	_run_syscall(syscall_id, 0x0);
	return;
};

void syscall_init(void)
{
	for (int32_t id = 0; id < MAX_SYSCALL; id++) {
		syscalls[id] = (syscall_entry_t){id, 0x0};
	};
	syscalls[SYS_EXIT].handler = _sys_exit;
	return;
};