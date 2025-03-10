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
	printf("==========================\n");
	printf("=   %s \n", name);
	printf("==========================\n");
	printf("st_dev: %d\n", self->st_dev);
	printf("st_mode: %d\n", self->st_mode);
	printf("st_size: %d\n", self->st_size);
	printf("st_blksize: %d\n", self->st_blksize);
	printf("st_blocks: %d\n", self->st_blocks);
	printf("st_atime: 0x%x\n", self->st_atime);
	printf("st_mtime: 0x%x\n", self->st_mtime);
	printf("st_ctime: 0x%x\n", self->st_ctime);
	printf("==========================\n");
	return;
};