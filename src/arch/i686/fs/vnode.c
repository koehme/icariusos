/**
 * @file vnode.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vnode.h"

void vnode_dump(const VStat *vstat, const char name[])
{
    kprtf("==========================\n");
    kprtf("=   %s \n", name);
    kprtf("==========================\n");
    kprtf("st_dev: %d\n", vstat->st_dev);
    kprtf("st_mode: %d\n", vstat->st_mode);
    kprtf("st_size: %d\n", vstat->st_size);
    kprtf("st_blksize: %d\n", vstat->st_blksize);
    kprtf("st_blocks: %d\n", vstat->st_blocks);
    kprtf("st_atime: 0x%x\n", vstat->st_atime);
    kprtf("st_mtime: 0x%x\n", vstat->st_mtime);
    kprtf("st_ctime: 0x%x\n", vstat->st_ctime);
    kprtf("==========================\n");
    return;
};