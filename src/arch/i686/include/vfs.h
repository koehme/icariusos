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

/**
 * @brief Function pointer for resolving operations in the file system.
 * Represents the function signature for resolving operations
 * on the file system, taking an ATADev pointer as a parameter.
 * @param dev Pointer to the ata device.
 * @return An integer representing the result of the resolve operation.
 */
typedef int (*ResolveFunction)(ATADev *dev);
/**
 * @brief Function pointer type for opening files in the file system.
 * Represents the function signature for opening files
 * in the file system, taking an ATADev pointer, a PathNode pointer,
 * and a VNODE_MODE parameter as parameters.
 * @param dev Pointer to the ATADev structure.
 * @param path Pointer to the PathNode structure representing the file path.
 * @param mode VNODE_MODE representing the open mode.
 */
typedef void *(*OpenFunction)(ATADev *dev, PathNode *path, const VNODE_MODE mode);

/**
 * @brief Represents the concrete file system superblock.
 * Holds function pointers for resolving and opening
 * operations in the file system, along with a character array for the
 * file system name.
 */
typedef struct Superblock
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    char name[10];
} Superblock;

/**
 * @struct FileDescriptor
 * @brief Is used to refer to the handle that the kernel returns to a program when it opens a file.
 */
typedef struct FileDescriptor
{
    int index;
    Superblock *fs;
    void *internal;
    ATADev *dev;
} FileDescriptor;

void vfs_init();
void vfs_insert(Superblock *fs);
int vfs_fopen(const char *file_name, const VNODE_MODE mode);
Superblock *vfs_resolve(ATADev *dev);

#endif