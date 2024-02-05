/**
 * @file fat16.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

#include "ata.h"
#include "vfs.h"

typedef enum FAT16Values
{
    FAT16_VALUE_FREE = 0x0000,
    FAT16_VALUE_RESERVED = 0x0001,
    FAT16_VALUE_BAD_CLUSTER = 0xFFF7,
    FAT16_VALUE_END_OF_CHAIN = 0xFFF8,
} FAT16Values;

typedef enum FAT16Limits
{
    FAT16_MIN_CLUSTERS = 4085,
    FAT16_MAX_CLUSTERS = 65525,
} FAT16Limits;

typedef enum FAT16FileAttributes
{
    READ_WRITE = 0x00,
    READ_ONLY = 0x01,
    HIDDEN = 0x02,
    SYSTEM = 0x04,
    VOLUME_ID = 0x08,
    DIRECTORY = 0x10,
    ARCHIVE = 0x20,
    LFN = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID,
    DEVICE = 0x40,
    DELETED = 0xE5,
} FAT16FileAttributes;

typedef struct FAT16
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    ReadFunction read_cb;
    char name[10];
} FAT16;

FileSystem *fat16_init(void);
int fat16_resolve(ATADev *dev);
void *fat16_open(ATADev *dev, PathNode *path, const VNODE_MODE mode);
size_t fat16_read(ATADev *dev, void *internal, uint8_t *buffer, size_t n_bytes, size_t n_blocks);

#endif