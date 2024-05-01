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
size_t fat16_read(ATADev *dev, void *internal, uint8_t *buffer, const size_t n_bytes, const size_t n_blocks);
int32_t fat16_close(void *internal);
int32_t fat16_stat(ATADev *dev, void *internal, VStat *vstat);
int32_t fat16_seek(void *internal, uint32_t offset, const VNODE_SEEK_MODE mode);

#endif