/**
 * @file superblock.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stddef.h>
#include <stdbool.h>

#include "ata.h"
#include "pparser.h"
#include "vnode.h"
#include "kernel.h"

typedef enum VFSLimit
{
    MAX_FS = 8,
    MAX_VNODE_DESCRIPTORS = 512
} VFSLimit;

/**
 * @brief Function pointer for esolving operations in the file system.
 * Represents the function signature for resolving operations
 * on the file system, taking an ATADisk pointer as a parameter.
 * @param disk Pointer to the ATADisk structure.
 * @return An integer representing the result of the resolve operation.
 */
typedef int (*ResolveFunction)(ATADisk *disk);
/**
 * @brief Function pointer type for opening files in the file system.
 * Represents the function signature for opening files
 * in the file system, taking an ATADisk pointer, a PathNode pointer,
 * and a VNODE_MODE parameter as parameters.
 * @param disk Pointer to the ATADisk structure.
 * @param path Pointer to the PathNode structure representing the file path.
 * @param mode VNODE_MODE representing the open mode.
 */
typedef void (*OpenFunction)(ATADisk *disk, PathNode *path, const VNODE_MODE mode);

/**
 * @brief Represents the concrete file system superblock.
 * Holds function pointers for resolving and opening
 * operations in the file system, along with a character array for the
 * file system name.
 */
typedef struct Superblock
{
    ResolveFunction s_resolve;
    OpenFunction s_open;
    char fs_name[10];
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
    ATADisk *disk;
} FileDescriptor;

void vfs_init();
void vfs_insert(Superblock *fs);
int fopen(const char *filename, const char *mode);
Superblock *vfs_resolve(ATADisk *disk);

#endif