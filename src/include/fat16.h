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

FileSystem* fat16_init(void);

int32_t fat16_resolve(ata_t* dev);
void* fat16_open(ata_t* dev, PathNode* path, const VNODE_MODE mode);
size_t fat16_read(ata_t* dev, void* internal, uint8_t* buffer, const size_t n_bytes, const size_t n_blocks);
int32_t fat16_close(void* internal);
int32_t fat16_stat(ata_t* dev, void* internal, VStat* vstat);
int32_t fat16_seek(void* internal, uint32_t offset, const VNODE_SEEK_MODE mode);

#endif