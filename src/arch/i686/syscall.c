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

static syscall_entry_t syscalls[MAX_SYSCALL] = {
    {
	0x0,
	0x0,
    },
};

const char* _get_syscall_name(const int32_t syscall_id)
{
	switch (syscall_id) {
	case SYS_EXIT:
		return "SYS_EXIT";
	case SYS_FORK:
		return "SYS_FORK";
	case SYS_EXECVE:
		return "SYS_EXECVE";
	case SYS_WAITPID:
		return "SYS_WAITPID";
	case SYS_GETPID:
		return "SYS_GETPID";
	case SYS_OPEN:
		return "SYS_OPEN";
	case SYS_CLOSE:
		return "SYS_CLOSE";
	case SYS_READ:
		return "SYS_READ";
	case SYS_WRITE:
		return "SYS_WRITE";
	case SYS_LSEEK:
		return "SYS_LSEEK";
	case SYS_UNLINK:
		return "SYS_UNLINK";
	case SYS_STAT:
		return "SYS_STAT";
	case SYS_FSTAT:
		return "SYS_FSTAT";
	case SYS_DUP:
		return "SYS_DUP";
	case SYS_DUP2:
		return "SYS_DUP2";
	case SYS_GETCWD:
		return "SYS_GETCWD";
	case SYS_CHDIR:
		return "SYS_CHDIR";
	case SYS_MKDIR:
		return "SYS_MKDIR";
	case SYS_RMDIR:
		return "SYS_RMDIR";
	case SYS_SBRK:
		return "SYS_SBRK";
	case SYS_TIME:
		return "SYS_TIME";
	case SYS_SLEEP:
		return "SYS_SLEEP";
	case SYS_YIELD:
		return "SYS_YIELD";
	case SYS_IOCTL:
		return "SYS_IOCTL";
	case SYS_REBOOT:
		return "SYS_REBOOT";
	default:
		return "Unknown Syscall";
	};
	return 0x0;
};

static void _sys_exit(const int32_t code)
{
	printf("[INFO] Usermode Task EXITED with %d! Back to Kernel-Land (Ring 0)\n", code);
	return;
};

static void _run_syscall(const int32_t syscall_id, const int32_t arg)
{
	if (syscall_id < 0 || syscall_id >= MAX_SYSCALL) {
		printf("[ERROR] Invalid Syscall %s - [%d]\n", _get_syscall_name(syscall_id), syscall_id);
		return;
	};
	const syscall_handler_t handler = syscalls[syscall_id].handler;

	if (!handler) {
		printf("[ERROR] Unimplemented Syscall: %s - [%d]\n", _get_syscall_name(syscall_id), syscall_id);
	} else {
		handler(arg);
	};
	kernel_shell();
	return;
};

void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame)
{
	printf("=====================================\n");
	printf("  Syscall Dispatcher: %s  - [%d]\n", _get_syscall_name(syscall_id), syscall_id);
	printf("=====================================\n");
	idt_dump_interrupt_frame(frame);
	page_restore_kernel_dir();
	task_save(frame);
	asm_restore_kernel_segment();

	_run_syscall(syscall_id, 0x0);
	return;
};

void syscall_init(void)
{
	for (int32_t id = 0; id < MAX_SYSCALL; id++) {
		const syscall_entry_t stub = {id, 0x0};
		syscalls[id] = stub;
	};
	syscalls[SYS_EXIT].handler = _sys_exit;
	return;
};