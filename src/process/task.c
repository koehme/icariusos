/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "icarius.h"
#include "page.h"
#include "string.h"

extern pfa_t pfa;

/* PUBLIC API */
void task_block_on(task_t* self, const wait_reason_t reason);
void task_exit(task_t* self);
task_t* task_create(process_t* parent, const uint8_t* file);
void task_dump(task_t* self);
int32_t task_get_stack_arg_at(int32_t i, interrupt_frame_t* frame);
void task_set_curr(task_t* self);
task_t* task_get_curr(void);
void task_set_block(task_t* self);
void task_set_unblock(task_t* self);
void task_switch(task_t* next);

/* INTERNAL API */
static task_t* _init_task(process_t* parent);
void task_restore_dir(task_t* self);
static void _load_binary_into_task(const uint8_t* file);
static task_t* curr_task = 0x0;

void task_block_on(task_t* self, const wait_reason_t reason)
{
	if (!self) {
		return;
	};
	self->waiting_on = reason;
	return;
};

void task_set_block(task_t* self)
{
	if (self) {
		self->state = TASK_STATE_BLOCK;
	};
	return;
};

void task_set_unblock(task_t* self)
{
	if (self && self->state == TASK_STATE_BLOCK) {
		self->waiting_on = WAIT_NONE;
		self->state = TASK_STATE_READY;
	};
	return;
};

void task_exit(task_t* self)
{
	if (!self) {
		return;
	};
	process_t* parent = self->parent;
	printf("[TASK] Task 0x%x exited. Remaining Tasks: %d\n", self, parent->task_count - 1);

	for (size_t i = 0; i < PROCESS_MAX_THREAD; i++) {
		if (parent->tasks[i] == self) {
			parent->tasks[i] = 0x0;
			break;
		};
	};
	self->parent->task_count--;
	return;
};

void task_set_curr(task_t* self)
{
	if (!self) {
		errno = EINVAL;
		return;
	};
	curr_task = self;
	return;
};

task_t* task_get_curr(void)
{
	if (!curr_task) {
		errno = ESRCH;
		return 0x0;
	};
	return curr_task;
};

void task_switch(task_t* next)
{
	if (!next) {
		return;
	};
	next->state = TASK_STATE_RUN;

	task_set_curr(next);
	process_set_curr(curr_task->parent);

	if (next->parent->page_dir) {
		task_restore_dir(next);
	};
	/*
	const char* str = (next->parent->filetype == PROCESS_KERNEL_THREAD) ? "[KTHREAD]" : "[UTHREAD]";

	printf("%s Switching to Task '%s' (PID %d)\n", str, next->parent->filename, next->parent->pid);
	size_t delay = 500000000;

	while (delay--) {
		;
	};
	*/
	asm_enter_task(&next->registers);
	return;
};

static task_t* _init_task(process_t* parent)
{
	task_t* task = kzalloc(sizeof(task_t));

	if (!task) {
		errno = ENOMEM;
		return 0x0;
	};
	memset(task, 0x0, sizeof(task_t));
	task->parent = parent;
	return task;
};

static void _load_binary_into_task(const uint8_t* file)
{
	const int32_t fd = vfs_fopen((char*)file, "r");
	vstat_t stat_buf = {};
	vfs_fstat(fd, &stat_buf);

	uint8_t* buf = kzalloc(stat_buf.st_size);
	const int32_t bytes_read = vfs_fread(buf, stat_buf.st_size, 1, fd);
	/*
	printf("[DEBUG] Read %d Bytes from %s\n", bytes_read, file);

	for (size_t i = 0; i < stat_buf.st_size; i++) {
		const uint8_t byte = buf[i];
		const uint8_t high = (byte >> 4) & 0x0F;
		const uint8_t low = byte & 0x0F;
		printf("0x%x%x ", high, low);
	};
	printf("\n");
	*/
	memcpy((void*)USER_CODE_START, (void*)buf, bytes_read);

	if (memcmp((void*)USER_CODE_START, (void*)buf, bytes_read) == 0) {
		printf("[SUCCESS] Usercode copied to Userspace at 0x%x\n", USER_CODE_START);
	} else {
		printf("[ERROR] Usercode copy failed.\n");
	};
	kfree(buf);
	return;
};

task_t* task_kcreate(process_t* parent, void (*entry)())
{
	if (parent->task_count >= PROCESS_MAX_THREAD) {
		printf("[ERROR] Max. Threads %d reached\n", PROCESS_MAX_THREAD);
		return 0x0;
	};
	task_t* task = _init_task(parent);

	if (!task) {
		errno = -ENOMEM;
		return 0x0;
	};
	void* stack = kzalloc(KERNEL_STACK_SIZE); // 32 KiB

	if (!stack) {
		errno = -ENOMEM;
		return 0x0;
	};
	task->stack_top = (uintptr_t)stack + KERNEL_STACK_SIZE - 1;
	task->stack_bottom = (uintptr_t)stack;

	task->registers.eip = (uintptr_t)entry;
	task->registers.eflags = (EFLAGS_IF | EFLAGS_MBS);
	task->registers.esp = task->registers.ebp = task->stack_top;

	task->registers.cs = GDT_KERNEL_CODE_SEGMENT;
	task->registers.ss = GDT_KERNEL_DATA_SEGMENT;
	printf("[KERNEL TASK] Stack: 0x%x - 0x%x (ESP = 0x%x)\n", task->stack_bottom, task->stack_top, task->registers.esp);
	return task;
};

task_t* task_create(process_t* parent, const uint8_t* file)
{
	if (parent->task_count >= PROCESS_MAX_THREAD) {
		printf("[ERROR] Max. Threads %d reached\n", PROCESS_MAX_THREAD);
		return 0x0;
	};
	task_t* task = _init_task(parent);

	if (!task) {
		return 0x0;
	};

	if (!parent || !parent->page_dir) {
		printf("[ERROR] Invalid Process or has no valid Page Directory!\n");
		return 0x0;
	};
	const uint32_t offset = parent->task_count * (USER_STACK_SIZE / PROCESS_MAX_THREAD); // 256 KiB per Stack
	const uint32_t stack_top = USER_STACK_END - offset;
	const uint32_t stack_bottom = stack_top - (USER_STACK_SIZE / PROCESS_MAX_THREAD) + 1;
	task->stack_top = stack_top;
	task->stack_bottom = stack_bottom;
	const uint32_t flags = (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	page_map_between(parent->page_dir, stack_bottom, stack_top, flags);

	task_restore_dir(task);
	_load_binary_into_task(file);
	page_restore_kernel_dir();

	task->registers.eip = USER_CODE_START;
	task->registers.eflags = (EFLAGS_IF | EFLAGS_MBS);
	task->registers.esp = task->registers.ebp = task->stack_top;

	task->registers.cs = GDT_USER_CODE_SEGMENT | 3; // 0x1B
	task->registers.ss = GDT_USER_DATA_SEGMENT | 3; // 0x23
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

	printf("Task Page Directory: 0x%x\n", v2p(self->parent->page_dir));
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
	return;
};

int32_t task_get_stack_arg_at(int32_t i, interrupt_frame_t* frame)
{
	const uint32_t* stack = (uint32_t*)frame->esp;

	if ((uintptr_t)stack[i] < KERNEL_VIRTUAL_START) {
		printf("[SECURITY] Invalid Task Stack Access\n");
		return -EINVAL;
	};
	return stack[i];
};

void task_restore_dir(task_t* self)
{
	if (!self || !self->parent->page_dir) {
		printf("[ERROR] task_set_directory: Invalid task or missing page directory!\n");
		return;
	};
	const uint32_t phys_addr = (uint32_t)(v2p((void*)self->parent->page_dir));
	asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
	// printf("[DEBUG] Switching to Task Page Directory at 0x%x\n", (void*)phys_addr);
	return;
};