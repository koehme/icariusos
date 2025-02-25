/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "icarius.h"
#include "string.h"

extern pfa_t pfa;

/* PUBLIC API */
void task_exit(task_t* self);
task_t* task_create(const uint8_t* file);
void task_dump(task_t* self);
int32_t task_get_stack_arg_at(int32_t i, interrupt_frame_t* frame);
task_t* task_get_curr(void);
void task_start(task_t* task);
task_t* curr_task = 0x0;

/* INTERNAL API */
static task_t* _init_task(void);
static void _init_task_register(task_t* task);
void task_restore_dir(task_t* self);
static void _load_binary_into_task(const uint8_t* file);

void task_exit(task_t* self)
{
	if (!curr_task) {
		return;
	};
	printf("[TASK] Task->Page_Dir 0x%x exited. Cleaning up...\n", self->page_dir);
	uint32_t* dir = self->page_dir;

	for (uint32_t i = 0; i < 768; i++) {
		if (dir[i] & PAGE_PRESENT) {
			const uint32_t virt_addr = i * 0x400000;
			const uint32_t phys_addr = page_get_phys_addr(dir, virt_addr);
			page_unmap_dir(dir, virt_addr);
			const uint64_t frame = phys_addr / PAGE_SIZE;
			pfa_clear(&pfa, frame);
			printf("[DEBUG] Freeing Page: Virt=0x%x, Phys=0x%x at Frame %d\n", virt_addr, phys_addr, frame);
		};
	};
	const uint32_t phys_addr = (uint32_t)v2p((void*)self->page_dir);
	const uint64_t frame = phys_addr / PAGE_SIZE;
	page_unmap_dir(page_get_dir(), (uint32_t)self->page_dir);
	pfa_clear(&pfa, frame);
	printf("[DEBUG] Freeing Page Directory: Virt=0x%x, Phys=0x%x at Frame %d\n", self->page_dir, phys_addr, frame);

	kfree(self);
	pfa_dump(&pfa, false);

	return;
};

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

static void _load_binary_into_task(const uint8_t* file)
{
	const int32_t fd = vfs_fopen((char*)file, "r");
	vstat_t stat_buf = {};
	vfs_fstat(fd, &stat_buf);

	uint8_t* buf = kzalloc(stat_buf.st_size);
	const int32_t bytes_read = vfs_fread(buf, stat_buf.st_size, 1, fd);
	printf("[DEBUG] Read %d Bytes from SHELL.BIN\n", bytes_read);

	for (size_t i = 0; i < stat_buf.st_size; i++) {
		const uint8_t byte = buf[i];
		const uint8_t high = (byte >> 4) & 0x0F;
		const uint8_t low = byte & 0x0F;
		printf("0x%x%x ", high, low);
	};
	printf("\n");

	memcpy((void*)USER_CODE_START, (void*)buf, bytes_read);

	if (memcmp((void*)USER_CODE_START, (void*)buf, bytes_read) == 0) {
		printf("[SUCCESS] Usercode copied to Userspace at 0x%x\n", USER_CODE_START);
	} else {
		printf("[ERROR] Usercode copy failed.\n");
	};
	kfree(buf);
	return;
};

void task_start(task_t* task)
{
	if (!task)
		return;
	printf("[TASK] Starting Task 0x%x\n", task);
	task_restore_dir(task);
	asm_enter_usermode(&task->registers);
	return;
};

task_t* task_create(const uint8_t* file)
{
	task_t* task = _init_task();

	if (!task) {
		errno = ENOMEM;
		return 0x0;
	};
	const uint32_t flags = (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	task->page_dir = page_create_dir(flags);
	page_map_between(task->page_dir, USER_CODE_START, USER_BSS_END, flags);
	page_map_between(task->page_dir, USER_HEAP_START, USER_HEAP_END, flags);
	page_map_between(task->page_dir, USER_STACK_START, USER_STACK_END, flags);

	task_restore_dir(task);

	_load_binary_into_task(file);

	page_restore_kernel_dir();
	_init_task_register(task);

	// task_restore_dir(task);
	// asm_enter_usermode(&task->registers);

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
	// printf("[TASK] Saving Task State for 0x%x\n", task);
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
	// task_dump(task);
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
	if (!self || !self->page_dir) {
		printf("[ERROR] task_set_directory: Invalid task or missing page directory!\n");
		return;
	};
	const uint32_t phys_addr = (uint32_t)(v2p((void*)self->page_dir));
	asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
	// printf("[DEBUG] Switching to Task Page Directory at 0x%x\n", (void*)phys_addr);
	return;
};