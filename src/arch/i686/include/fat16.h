/**
 * @file fat16.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_H
#define FAT16_H

#define FAT16_DEBUG_DELAY 0

#include <stdint.h>

#include "ata.h"
#include "vfs.h"

FileSystem *fat16_init(void);

int32_t fat16_resolve(ATADev *dev);
void *fat16_open(ATADev *dev, PathNode *path, const VNODE_MODE mode);
size_t fat16_read(ATADev *dev, void *internal, uint8_t *buffer, size_t n_bytes, size_t n_blocks);

#endif