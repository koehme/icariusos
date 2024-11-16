/**
 * @file vnode.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vnode.h"
#include "stdio.h"

void vnode_dump(const VStat* vstat, const char name[])
{
	printf("==========================\n");
	printf("=   %s \n", name);
	printf("==========================\n");
	printf("st_dev: %d\n", vstat->st_dev);
	printf("st_mode: %d\n", vstat->st_mode);
	printf("st_size: %d\n", vstat->st_size);
	printf("st_blksize: %d\n", vstat->st_blksize);
	printf("st_blocks: %d\n", vstat->st_blocks);
	printf("st_atime: 0x%x\n", vstat->st_atime);
	printf("st_mtime: 0x%x\n", vstat->st_mtime);
	printf("st_ctime: 0x%x\n", vstat->st_ctime);
	printf("==========================\n");
	return;
};