/**
 * @file vfs.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdbool.h>

#include "pathparser.h"
#include "vnode.h"
#include "kernel.h"

typedef struct ATADev ATADev;

typedef int (*ResolveFunction)(ATADev *dev);
typedef void *(*OpenFunction)(ATADev *dev, PathNode *path, const VNODE_MODE mode);
typedef size_t (*ReadFunction)(ATADev *dev, void *internal, uint8_t *buffer, size_t n_bytes, size_t n_blocks);

typedef struct Superblock
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    ReadFunction read_cb;
    char name[10];
} Superblock;

typedef struct FileDescriptor
{
    int index;
    Superblock *fs;
    void *internal;
    ATADev *dev;
} FileDescriptor;

void vfs_init();
void vfs_insert(Superblock *fs);
int vfs_fopen(const char *filename, const char *mode);
size_t vfs_fread(void *buffer, size_t n_bytes, size_t n_blocks, int32_t fd_index);
Superblock *vfs_resolve(ATADev *dev);

#endif