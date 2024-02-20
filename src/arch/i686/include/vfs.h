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

typedef int32_t (*ResolveFunction)(ATADev *dev);
typedef void *(*OpenFunction)(ATADev *dev, PathNode *path, const VNODE_MODE mode);
typedef size_t (*ReadFunction)(ATADev *dev, void *internal, uint8_t *buffer, size_t n_bytes, size_t n_blocks);
typedef int32_t (*CloseFunction)(void *internal);

typedef struct FileSystem
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    ReadFunction read_cb;
    CloseFunction close_cb;
    char name[10];
} FileSystem;

typedef struct FileDescriptor
{
    int32_t index;
    FileSystem *fs;
    void *internal;
    ATADev *dev;
} FileDescriptor;

void vfs_init();
void vfs_insert(FileSystem *fs);
int32_t vfs_fopen(const char *filename, const char *mode);
size_t vfs_fread(void *buffer, size_t n_bytes, size_t n_blocks, const int32_t fd);
FileSystem *vfs_resolve(ATADev *dev);
int32_t vfs_fclose(const int32_t fd);

#endif