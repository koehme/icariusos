/**
 * @file vnode.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vnode.h"
#include "stdio.h"

/* PUBLIC API */
void vnode_dump(const vstat_t* self, const char name[]);

void vnode_dump(const vstat_t* self, const char name[])
{
	kprintf("==========================\n");
	kprintf("=   %s \n", name);
	kprintf("==========================\n");
	kprintf("st_dev: %d\n", self->st_dev);
	kprintf("st_mode: %d\n", self->st_mode);
	kprintf("st_size: %d\n", self->st_size);
	kprintf("st_blksize: %d\n", self->st_blksize);
	kprintf("st_blocks: %d\n", self->st_blocks);
	kprintf("st_atime: 0x%x\n", self->st_atime);
	kprintf("st_mtime: 0x%x\n", self->st_mtime);
	kprintf("st_ctime: 0x%x\n", self->st_ctime);
	kprintf("==========================\n");
	return;
};