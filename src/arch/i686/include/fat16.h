/**
 * @file fat16.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

#include "ata.h"
#include "superblock.h"

typedef struct FAT16
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    char name[MAX_FS_NAME_LENGTH];
} FAT16;

typedef struct Fat16_Base_Header
{
    uint8_t jmp_short[3];         // jump over the disk format information (the BPB and EBPB)
    uint8_t oem_ident[8];         // oem identifier. The first 8 Bytes (3 - 10) is the version of DOS being used
    uint16_t bytes_per_sector;    // number of Bytes per sector (remember, all numbers are in the little-endian format)
    uint8_t sectors_per_cluster;  // number of sectors per cluster
    uint16_t reserved_sectors;    // number of reserved sectors. The boot record sectors are included in this value
    uint8_t fat_table_copies;     // number of File Allocation Tables (FAT's) on the storage media
    uint16_t root_directories;    // number of root directory entries (must be set so that the root directory occupies entire sectors)
    uint16_t total_sectors;       // total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20
    uint8_t media_desc_type;      // indicates the media descriptor type
    uint16_t sectors_per_fat;     // number of sectors per FAT
    uint16_t sectors_per_track;   // number of sectors per track
    uint16_t heads;               // number of heads or sides on the storage media
    uint32_t hidden_sectors;      // number of hidden sectors. (i.e. the LBA of the beginning of the partition
    uint32_t large_total_sectors; // large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not fit in the Number of Sectors entry at 0x13
} __attribute__((packed)) Fat16_Base_Header;

/*
The extended boot record information comes right after the BPB.
The data at the beginning is known as the EBPB.
It contains different information depending on whether this partition is a FAT 12, FAT 16, or FAT 32 filesystem.
Immediately following the EBPB is the actual boot code, then the standard 0xAA55 boot signature, to fill out the 512-byte boot sector. Offsets shows are from the start of the standard boot record.
*/
typedef struct Fat16_Extended_Header
{
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t system_ident[8];
} __attribute__((packed)) Fat16_Extended_Header;

typedef struct Fat16_Header
{
    Fat16_Base_Header base;
    union
    {
        Fat16_Extended_Header extended;
    } optional;
} Fat16_Header;

Superblock *fat16_init(void);
int fat16_resolve(ATADisk *disk);
void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode);

#endif