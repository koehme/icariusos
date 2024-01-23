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

typedef struct FAT16BaseHeader
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
} __attribute__((packed)) FAT16BaseHeader;

/*
The extended boot record information comes right after the BPB.
The data at the beginning is known as the EBPB.
It contains different information depending on whether this partition is a FAT 12, FAT 16, or FAT 32 filesystem.
Immediately following the EBPB is the actual boot code, then the standard 0xAA55 boot signature, to fill out the 512-byte boot sector. Offsets shows are from the start of the standard boot record.
*/
typedef struct FAT16ExtendedHeader
{
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t system_ident[8];
} __attribute__((packed)) FAT16ExtendedHeader;

typedef enum FAT16FileAttributes
{
    READ_ONLY = 0x01,
    HIDDEN = 0x02,
    SYSTEM = 0x04,
    VOLUME_ID = 0x08,
    DIRECTORY = 0x10,
    ARCHIVE = 0x20,
    LFN = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID
} FAT16FileAttributes;

typedef struct FAT16DirectoryEntry
{
    uint8_t file_name[11];      // 8.3 file name
    uint8_t attributes;         // attributes of the file
    uint8_t reserved;           // reserved for use by Windows NT
    uint8_t creation_time_ms;   // creation time in hundredths of a second
    uint16_t create_time;       // time that the file was created (in the format described)
    uint16_t create_date;       // date on which the file was created (in the format described)
    uint16_t last_access_date;  // last accessed date (in the format described)
    uint16_t high_cluster;      // high 16 bits of the first cluster number
    uint16_t modification_time; // last modification time (in the format described)
    uint16_t modification_date; // last modification date (in the format described)
    uint16_t low_cluster;       // low 16 bits of the first cluster number
    uint32_t file_size;         // size of the file in bytes
} __attribute__((packed)) FAT16DirectoryEntry;

typedef struct FAT16LongDirectoryEntry
{
    uint8_t ldir_ord;
    uint16_t ldir_name1[5];
    uint8_t ldir_attr;
    uint8_t ldir_type;
    uint8_t ldir_chksum;
    uint16_t ldir_name2[6];
    uint16_t ldir_fstcluslo;
    uint16_t ldir_name3[2];
} __attribute__((packed)) FAT16LongDirectoryEntry;

typedef struct FAT16InternalHeader
{
    FAT16BaseHeader base;
    FAT16ExtendedHeader ext;
} __attribute__((packed)) FAT16InternalHeader;

typedef struct FAT16TimeInfo
{
    int hour;
    int minute;
    int second;
} FAT16TimeInfo;

typedef struct FAT16DateInfo
{
    int day;
    int month;
    int year;
} FAT16DateInfo;

Superblock *fat16_init(void);
int fat16_resolve(ATADisk *disk);
void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode);

#endif