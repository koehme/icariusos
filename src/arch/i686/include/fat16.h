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

typedef struct FAT16
{
    ResolveFunction resolve_cb;
    OpenFunction open_cb;
    char name[10];
} FAT16;

typedef struct BIOSParameterBlock
{
    uint8_t BS_jmpBoot[3];   // jump over the disk format information (the BPB and EBPB)
    uint8_t BS_OEMName[8];   // oem identifier. The first 8 Bytes (3 - 10) is the version of DOS being used
    uint16_t BPB_BytsPerSec; // number of Bytes per sector (remember, all numbers are in the little-endian format)
    uint8_t BPB_SecPerClus;  // number of sectors per cluster
    uint16_t BPB_RsvdSecCnt; // number of reserved sectors. The boot record sectors are included in this value
    uint8_t BPB_NumFATs;     // number of File Allocation Tables (FAT's) on the storage media
    uint16_t BPB_RootEntCnt; // number of root directory entries (must be set so that the root directory occupies entire sectors)
    uint16_t BPB_TotSec16;   // total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20
    uint8_t BPB_Media;       // indicates the media descriptor type
    uint16_t BPB_FATSz16;    // number of sectors per FAT
    uint16_t BPB_SecPerTrk;  // number of sectors per track
    uint16_t BPB_NumHeads;   // number of heads or sides on the storage media
    uint32_t BPB_HiddSec;    // number of hidden sectors. (i.e. the LBA of the beginning of the partition
    uint32_t BPB_TotSec32;   // large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not fit in the Number of Sectors entry at 0x13
} __attribute__((packed)) BIOSParameterBlock;

/*
The extended boot record information comes right after the BPB.
The data at the beginning is known as the EBPB.
It contains different information depending on whether this partition is a FAT 12, FAT 16, or FAT 32 filesystem.
Immediately following the EBPB is the actual boot code, then the standard 0xAA55 boot signature, to fill out the 512-byte boot sector. Offsets shows are from the start of the standard boot record.
*/
typedef struct ExtendedBIOSParameterBlock
{
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
} __attribute__((packed)) ExtendedBIOSParameterBlock;

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

typedef struct FAT16FileDescriptor
{
    FAT16DirectoryEntry *entry;     // Pointer to the directory entry of the file
    int total;                      // Total number of sectors or clusters (depending on the implementation)
    int sector_pos;                 // Current position of the file descriptor in the file system (sector or cluster)
    int ending_sector_pos;          // End position of the file in the file system (sector or cluster)
    uint32_t pos;                   // Current position within the file (in bytes)
    FAT16FileAttributes attributes; // File attributes (e.g., READ_ONLY, DIRECTORY)
} FAT16FileDescriptor;

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
    BIOSParameterBlock bpb;
    ExtendedBIOSParameterBlock ebpb;
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
int fat16_resolve(ATADev *dev);
void *fat16_open(ATADev *dev, PathNode *path, const VNODE_MODE mode);

#endif