/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "icarius.h"
#include "string.h"

/* PUBLIC API */
task_t* task_create(void (*user_eip)());
task_t* curr_task = 0x0;

/* INTERNAL API */
static task_t* _init_task(void);
static void _init_task_register(task_t* task);
static void _set_task_dir(task_t* self);

task_t* task_get_curr(void)
{
	if (!curr_task) {
		printf("[ERROR] No current Task are available!\n");
		return 0x0;
	};
	return curr_task;
};

static task_t* _init_task(void)
{
	task_t* task = kzalloc(sizeof(task_t));

	if (!task) {
		errno = ENOMEM;
		return 0x0;
	};
	memset(task, 0x0, sizeof(task_t));
	return task;
};

static void _init_task_register(task_t* task)
{
	if (!task) {
		printf("[ERROR] Invalid Task!\n");
		return;
	};
	task->registers.eip = USER_CODE_START;
	task->registers.eflags = 0x200;
	task->registers.esp = task->registers.ebp = USER_STACK_END;

	task->registers.cs = GDT_USER_CODE_SEGMENT | 3; // 0x1B
	task->registers.ss = GDT_USER_DATA_SEGMENT | 3; // 0x23

	curr_task = task;

	task_dump(task);

	return;
};

task_t* task_create(void (*user_eip)())
{
	task_t* task = _init_task();

	if (!task) {
		errno = ENOMEM;
		return 0x0;
	};
	const uint32_t flags = (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	task->page_dir = page_create_dir(flags);

	/* Map code section at virtual addr 0x0 - 0x400000 in task page dir */
	page_map_between(task->page_dir, USER_CODE_START, (USER_CODE_START + PAGE_SIZE), flags);
	/* Map code section at virtual addr 0xBFC00000 - 0xBFFFFFFF in task page dir */
	page_map_between(task->page_dir, USER_STACK_START, USER_STACK_END, flags);

	_set_task_dir(task);
	memcpy((void*)USER_CODE_START, (void*)user_eip, 1024);

	if (memcmp((void*)USER_CODE_START, (void*)user_eip, 1024) == 0) {
		printf("[SUCCESS] User Code copied to Userspace at 0x%x\n", USER_CODE_START);
	} else {
		printf("[ERROR] User Code copy failed!\n");
	};
	page_restore_kernel_dir();
	_init_task_register(task);

	_set_task_dir(task);
	asm_enter_usermode(&task->registers);

	return task;
};

void task_dump(task_t* self)
{
	if (!self) {
		printf("[ERROR] Invalid Task Pointer!\n");
		return;
	};
	printf("\n");
	printf("====================================\n");
	printf("          TASK STATE DUMP           \n");
	printf("====================================\n");

	printf("Task Page Directory: 0x%x\n", v2p(self->page_dir));
	printf("EIP  (Instruction Pointer) : 0x%x\n", self->registers.eip);
	printf("ESP  (Stack Pointer)       : 0x%x\n", self->registers.esp);
	printf("EBP  (Base Pointer)        : 0x%x\n", self->registers.ebp);
	printf("EFLAGS                     : 0x%x\n", self->registers.eflags);
	printf("CS   (Code Segment)        : 0x%x\n", self->registers.cs);
	printf("SS   (Stack Segment)       : 0x%x\n", self->registers.ss);

	printf("------------------------------------\n");
	printf("General Purpose Registers:\n");
	printf("EAX  : 0x%x\n", self->registers.eax);
	printf("EBX  : 0x%x\n", self->registers.ebx);
	printf("ECX  : 0x%x\n", self->registers.ecx);
	printf("EDX  : 0x%x\n", self->registers.edx);
	printf("ESI  : 0x%x\n", self->registers.esi);
	printf("EDI  : 0x%x\n", self->registers.edi);

	printf("====================================\n");
	return;
};

void task_save(interrupt_frame_t* frame)
{
	task_t* task = task_get_curr();

	if (!task) {
		printf("[ERROR] No current Task to SAVE!\n");
		return;
	};
	printf("[TASK] Saving Task State for 0x%x\n", task);
	task->registers.edi = frame->edi;
	task->registers.esi = frame->esi;
	task->registers.ebp = frame->ebp;
	task->registers.ebx = frame->ebx;
	task->registers.edx = frame->edx;
	task->registers.ecx = frame->ecx;
	task->registers.eax = frame->eax;

	task->registers.eip = frame->eip;
	task->registers.cs = frame->cs;
	task->registers.eflags = frame->eflags;
	task->registers.esp = frame->esp;
	task->registers.ss = frame->ss;
	task_dump(task);
	return;
};

static void _set_task_dir(task_t* self)
{
	if (!self || !self->page_dir) {
		printf("[ERROR] task_set_directory: Invalid task or missing page directory!\n");
		return;
	};
	const uint32_t phys_addr = (uint32_t)(v2p((void*)self->page_dir));
	asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
	printf("[DEBUG] Switching to Task Page Directory at 0x%x\n", (void*)phys_addr);
	return;
};