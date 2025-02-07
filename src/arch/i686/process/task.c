/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "icarius.h"
#include "string.h"

task_t* task_head = 0x0;
task_t* task_tail = 0x0;
task_t* curr_task = 0x0;

int32_t task_init(task_t* self)
{
	memset(self, 0x0, sizeof(task_t));
	self->page_dir = page_create_directory(PAGE_PS | PAGE_WRITABLE | PAGE_PRESENT);

	if (!self->page_dir) {
		return -EIO;
	};
	self->registers.eip = USER_CODE_START;	    // 0x00000000
	self->registers.ss = GDT_USER_DATA_SEGMENT; // 0x20
	self->registers.esp = USER_STACK_END;	    // start: 0xBFC00000 - end: 0xBFFFFFFF
	return 0;
};

task_t* task_new(void)
{
	task_t* new_task = kzalloc(sizeof(task_t));

	if (!new_task) {
		errno = ENOMEM;
		return 0x0;
	};
	int32_t res = task_init(new_task);

	if (res != 0) {
		errno = -res;
		kfree((void*)new_task);
		return 0x0;
	};

	if (!task_head) {
		task_head = task_tail = new_task;
	} else {
		task_tail->next = new_task;
		new_task->prev = task_tail;
		task_tail = new_task;
	};
	return new_task;
};

task_t* task_create_user(void (*entry_point)())
{
	task_t* user_task = task_new();

	if (!user_task) {
		return 0x0;
	};
	const uint64_t user_code_phys = pfa_alloc();

	if (!user_code_phys) {
		return 0x0;
	};
	page_map_dir(user_task->page_dir, USER_CODE_START, user_code_phys, PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	memcpy((void*)p2v((uint32_t)user_code_phys), entry_point, 4096);

	user_task->registers.eip = USER_CODE_START;
	user_task->registers.cs = GDT_USER_CODE_SEGMENT;
	user_task->registers.eflags = 0x200;
	user_task->registers.esp = USER_STACK_END;
	user_task->registers.ss = GDT_USER_DATA_SEGMENT;

	printf("[DEBUG] Task Registers - CS: 0x%x, DS: 0x%x, EIP: 0x%x\n", user_task->registers.cs, user_task->registers.ss, user_task->registers.eip);

	return user_task;
};

task_t* task_get_curr(void) { return curr_task; };

void task_switch_page_directory(task_t* task)
{
	if (!task || !task->page_dir) {
		printf("[ERROR] Invalid Task or Page Directory!\n");
		return;
	};
	printf("[INFO] Switching to Task Page Directory: 0x%x\n", (uint32_t)task->page_dir);
	uint32_t phys_addr = (uint32_t)task->page_dir - KERNEL_VIRTUAL_START;
	printf("[INFO] Switching to Task Page Directory: Virt=0x%x, Phys=0x%x\n", (uint32_t)task->page_dir, phys_addr);
	page_set_directory((uint32_t*)phys_addr);
	printf("[DEBUG] Task Page Directory: Virt=0x%x, Phys=0x%x\n", (uint32_t)task->page_dir, (uint32_t)task->page_dir - KERNEL_VIRTUAL_START);
	return;
};