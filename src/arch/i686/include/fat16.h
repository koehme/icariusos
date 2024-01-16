/**
 * @file fat16.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_H
#define FAT16_H

#include "ata.h"
#include "superblock.h"

typedef struct FAT16
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    char name[MAX_FS_NAME_LENGTH];
} FAT16;

Superblock *fat16_init(void);
int fat16_resolve(ATADisk *disk);
void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode);

#endif