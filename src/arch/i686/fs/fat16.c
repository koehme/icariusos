/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16.h"
#include "stream.h"
#include "mem.h"

#define FAT16_DEBUG_DELAY 0

Superblock fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .name = "FAT16",
};

FAT16InternalHeader fat16_header = {
    .bpb = {
        .BS_jmpBoot = {0},
        .BS_OEMName = {0},
        .BPB_BytsPerSec = 0,
        .BPB_SecPerClus = 0,
        .BPB_RsvdSecCnt = 0,
        .BPB_NumFATs = 0,
        .BPB_RootEntCnt = 0,
        .BPB_TotSec16 = 0,
        .BPB_Media = 0,
        .BPB_FATSz16 = 0,
        .BPB_SecPerTrk = 0,
        .BPB_NumHeads = 0,
        .BPB_HiddSec = 0,
        .BPB_TotSec32 = 0,
    },
    .ebpb = {
        .BS_DrvNum = 0,
        .BS_Reserved1 = 0,
        .BS_BootSig = 0,
        .BS_VolID = 0,
        .BS_VolLab = {0},
        .BS_FilSysType = {0},
    },
};

static FAT16TimeInfo fat16_convert_time(uint16_t time)
{
    FAT16TimeInfo time_info;
    time_info.second = (time & 0x1F) * 2;
    time_info.minute = (time >> 5) & 0x3F;
    time_info.hour = (time >> 11) & 0x1F;
    return time_info;
};

static FAT16DateInfo fat16_convert_date(uint16_t date)
{
    FAT16DateInfo date_info;
    date_info.day = date & 0x1F;
    date_info.month = (date >> 5) & 0x0F;
    uint16_t original_year = (date >> 9) & 0x7F;
    date_info.year = (original_year == 0) ? 0 : (original_year + 1980);
    return date_info;
};

Superblock *fat16_init(void)
{
    fat16.resolve_cb = fat16_resolve;
    fat16.open_cb = fat16_open;
    return &fat16;
};

static bool fat16_validate_header(const FAT16InternalHeader *header)
{
    const bool has_signature = (header->bpb.BS_jmpBoot[0] == 0xEB && header->bpb.BS_jmpBoot[2] == 0x90);
    const bool has_header = has_signature &&
                            (header->bpb.BPB_BytsPerSec == 512) &&
                            (header->bpb.BPB_SecPerClus >= 1) &&
                            (header->bpb.BPB_NumFATs == 2) &&
                            (header->bpb.BPB_Media != 0);

    if (!has_header)
    {
        return false;
    };
    const bool has_ebpb = (header->ebpb.BS_BootSig == 0x29) && (header->ebpb.BS_DrvNum == 0x80);

    if (has_ebpb)
    {
        const uint32_t total_sectors = (header->bpb.BPB_TotSec32 != 0) ? header->bpb.BPB_TotSec32 : header->bpb.BPB_TotSec16;
        const uint32_t disk_size = total_sectors * header->bpb.BPB_BytsPerSec;

        if (total_sectors > disk_size)
        {
            return false;
        };
    };
    return true;
};

static void fat16_dump_ebpb_header(const ExtendedBIOSParameterBlock *ebpb, const char *msg, const int delay)
{
    uint8_t BS_VolLab[12] = {};
    uint8_t BS_FilSysType[9] = {};
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("BS_DrvNum: 0x%x\n", ebpb->BS_DrvNum);
    kprintf("BS_Reserved1: 0x%x\n", ebpb->BS_Reserved1);
    kprintf("BS_BootSig: %d\n", ebpb->BS_BootSig);
    kprintf("BS_VolID: 0x%x\n", ebpb->BS_VolID);
    mcpy(BS_VolLab, ebpb->BS_VolLab, 11);
    mcpy(BS_FilSysType, ebpb->BS_FilSysType, 7);
    kprintf("BS_VolLab: %s\n", BS_VolLab);
    kprintf("BS_FilSysType: %s\n", BS_FilSysType);
    kprintf("----------------------------------\n");
    kdelay(delay);
    return;
};

static void fat16_dump_base_header(const BIOSParameterBlock *bpb, const char *msg, const int delay)
{
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("BS_jmpBoot: 0x%x 0x%x\n", bpb->BS_jmpBoot[0], bpb->BS_jmpBoot[1], bpb->BS_jmpBoot[2]);
    kprintf("BS_OEMName: %s\n", bpb->BS_OEMName);
    kprintf("BPB_BytsPerSec: %d\n", bpb->BPB_BytsPerSec);
    kprintf("BPB_SecPerClus: %d\n", bpb->BPB_SecPerClus);
    kprintf("BPB_RsvdSecCnt: %d\n", bpb->BPB_RsvdSecCnt);
    kprintf("BPB_NumFATs: %d\n", bpb->BPB_NumFATs);
    kprintf("BPB_RootEntCnt: %d\n", bpb->BPB_RootEntCnt);
    kprintf("BPB_TotSec16: %d\n", bpb->BPB_TotSec16);
    kprintf("BPB_Media: 0x%x\n", bpb->BPB_Media);
    kprintf("BPB_FATSz16: %d\n", bpb->BPB_FATSz16);
    kprintf("BPB_SecPerTrk: %d\n", bpb->BPB_SecPerTrk);
    kprintf("BPB_NumHeads: %d\n", bpb->BPB_NumHeads);
    kprintf("BPB_HiddSec: %d\n", bpb->BPB_HiddSec);
    kprintf("BPB_TotSec32: %d\n", bpb->BPB_TotSec32);
    kprintf("----------------------------------\n");
    kdelay(delay);
    return;
};

static uint32_t calculate_root_dir_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t root_directory_offset = bpb->BPB_BytsPerSec * (bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz16));
    return root_directory_offset;
};

static uint32_t calculate_root_dir_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)

{
    const uint32_t root_directory_absolute = partition_offset + calculate_root_dir_offset(bpb);
    return root_directory_absolute;
};

static uint32_t calculate_fat_table_offset(const BIOSParameterBlock *bpb, const uint32_t partition_offset)
{
    const uint32_t fat_table_offset = partition_offset + bpb->BPB_RsvdSecCnt * bpb->BPB_BytsPerSec;
    return fat_table_offset;
};

static bool is_lfn_entry(uint8_t *entry)
{
    return entry[11] == LFN;
};

static uint8_t extract_lsb(uint16_t value)
{
    return ((uint8_t)value & 0x00FF);
};

static void print_fat16_lfn_entry(int i, FAT16LongDirectoryEntry *lfn_entry, const int delay)
{
    kprintf("==========================\n");
    kprintf("=   LFN Entry %d:\n", i);
    kprintf("==========================\n");

    for (int j = 0; j < 5; j++)
    {
        kprintf("%c", extract_lsb(lfn_entry->ldir_name1[j]));
    };
    for (int j = 0; j < 6; j++)
    {
        kprintf("%c", extract_lsb(lfn_entry->ldir_name2[j]));
    };
    for (int j = 0; j < 2; j++)
    {
        kprintf("%c", extract_lsb(lfn_entry->ldir_name3[j]));
    };
    kprintf("\n");
    kprintf("=   Attributes: 0x%x\n", lfn_entry->ldir_attr);
    kprintf("=   Type: 0x%x\n", lfn_entry->ldir_type);
    kprintf("=   Checksum: 0x%x\n", lfn_entry->ldir_chksum);
    kprintf("=   First Cluster Low: %d\n", lfn_entry->ldir_fstcluslo);
    kprintf("==========================\n");
    kdelay(delay);
    return;
};

static void print_fat16_dir_entry(int i, FAT16DirectoryEntry *entry, const int delay)
{
    FAT16TimeInfo create_time = fat16_convert_time(entry->create_time);
    FAT16DateInfo create_date = fat16_convert_date(entry->create_date);
    FAT16DateInfo last_access_date = fat16_convert_date(entry->last_access_date);
    FAT16TimeInfo mod_time = fat16_convert_time(entry->modification_time);
    FAT16DateInfo mod_date = fat16_convert_date(entry->modification_date);

    kprintf("==========================\n");
    kprintf("=   RootDirEntry %d:\n", i);
    kprintf("==========================\n");
    kprintf("=   Filename: %s\n", entry->file_name);
    kprintf("=   Attributes: 0x%x\n", entry->attributes);
    kprintf("=   Creation Time: %d:%d:%d\n", create_time.hour, create_time.minute, create_time.second);
    kprintf("=   Creation Date: %d.%d.%d\n", create_date.day, create_date.month, create_date.year);
    kprintf("=   Last Access Date: %d.%d.%d\n", last_access_date.day, last_access_date.month, last_access_date.year);
    kprintf("=   High Cluster: %d\n", entry->high_cluster);
    kprintf("=   Modification Time: %d:%d:%d\n", mod_time.hour, mod_time.minute, mod_time.second);
    kprintf("=   Modification Date: %d.%d.%d\n", mod_date.day, mod_date.month, mod_date.year);
    kprintf("=   Low Cluster: %d\n", entry->low_cluster);
    kprintf("=   File Size: %d Bytes\n", entry->file_size);
    kprintf("==========================\n");

    kdelay(delay);
    return;
};

static void fat16_dump_root_dir_entries(const BIOSParameterBlock *bpb, Stream *stream)
{
    const uint32_t root_dir_size = bpb->BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_entries = root_dir_size / sizeof(FAT16DirectoryEntry);

    for (int i = 0; i < root_dir_entries; i++)
    {
        FAT16DirectoryEntry entry = {};
        stream_read(stream, (uint8_t *)&entry, sizeof(FAT16DirectoryEntry));

        if (is_lfn_entry((uint8_t *)&entry))
        {
            print_fat16_lfn_entry(i, (FAT16LongDirectoryEntry *)&entry, FAT16_DEBUG_DELAY);
        }
        else
        {
            print_fat16_dir_entry(i, &entry, FAT16_DEBUG_DELAY);
        };
    };
    return;
};

int fat16_resolve(ATADisk *disk)
{
    Stream header_stream;
    const uint32_t partition_offset = 0x100000;
    stream_init(&header_stream, disk);
    stream_seek(&header_stream, partition_offset);
    const int res = stream_read(&header_stream, (uint8_t *)&fat16_header, sizeof(FAT16InternalHeader));

    if (res != 0)
    {
        kprintf("Error: Failed to read FAT16 Header.\n");
        return -EIO;
    };

    if (!fat16_validate_header(&fat16_header))
    {
        kprintf("Error: Invalid FAT16 Header.\n");
        return -EIO;
    };
    const uint32_t root_dir_offset = calculate_root_dir_offset(&fat16_header.bpb);
    const uint32_t root_dir_absolute = calculate_root_dir_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t root_dir_size = fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_entries = root_dir_size / sizeof(FAT16DirectoryEntry);

    kprintf("Root Directory Offset: 0x%x\n", root_dir_offset);
    kprintf("Root Directory Absolute: 0x%x\n", root_dir_absolute);
    kprintf("Root Directory Size: %d\n", root_dir_size);
    kprintf("Root Directory Entries: %d\n", root_dir_entries);

    fat16_dump_base_header(&fat16_header.bpb, "", 0);
    fat16_dump_ebpb_header(&fat16_header.ebpb, "", 0);

    Stream root_dir = {};
    stream_init(&root_dir, disk);
    stream_seek(&root_dir, root_dir_absolute);
    fat16_dump_root_dir_entries(&fat16_header.bpb, &root_dir);

    const uint32_t fat_table_offset = calculate_fat_table_offset(&fat16_header.bpb, partition_offset);
    const uint32_t fat_table_size_bytes = fat16_header.bpb.BPB_FATSz16 * fat16_header.bpb.BPB_BytsPerSec;
    const uint32_t fat_table_entries = fat_table_size_bytes / sizeof(uint16_t);

    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, VNODE_MODE mode)
{
    if (mode != V_READ)
    {
        return 0x0;
    };
    // Start from the root directory
    // Step 1: Define a structure to hold information about the root directory entry
    // Step 2: Read the root directory entry from the disk
    // Step 2a: Handle error - Unable to read directory entry
    // Step 3: Check if the entry corresponds to a file or a directory
    // Step 3a: Handle logic for opening a directory
    // For directories, you might perform operations like listing its contents
    // Additional logic specific to directories can be implemented here
    // Step 3b: Handle logic for opening a file
    // Step 4: Allocate memory for the structure to represent the file
    // Step 4a: Handle error - Unable to allocate memory for the file structure
    // Step 5: Initialize the file structure with information from the directory entry
    // Step 6: Additional logic for file opening can be added here
    // For example, completing the cluster chain, loading additional metadata, etc.
    // Step 7: Return the initialized file structure
    // End of the file or directory opening logic
    return 0x0;
};