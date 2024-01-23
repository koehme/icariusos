/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16.h"
#include "stream.h"
#include "mem.h"

Superblock fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .name = "FAT16",
};

FAT16InternalHeader fat16_header = {};

FAT16TimeInfo fat16_convert_time(uint16_t time)
{
    FAT16TimeInfo time_info;
    time_info.second = (time & 0x1F) * 2;
    time_info.minute = (time >> 5) & 0x3F;
    time_info.hour = (time >> 11) & 0x1F;
    return time_info;
};

FAT16DateInfo fat16_convert_date(uint16_t date)
{
    FAT16DateInfo date_info;
    date_info.day = date & 0x1F;
    date_info.month = (date >> 5) & 0x0F;
    date_info.year = ((date >> 9) & 0x7F) + 1980;
    return date_info;
};

Superblock *fat16_init(void)
{
    fat16.resolve_cb = fat16_resolve;
    fat16.open_cb = fat16_open;
    return &fat16;
};

bool fat16_validate_header(const BIOSParameterBlock *bpb)
{
    return (bpb->BS_jmpBoot[0] == 0xEB && bpb->BS_jmpBoot[2] == 0x90) &&
           (bpb->BPB_BytsPerSec == 512) &&
           (bpb->BPB_SecPerClus >= 1) &&
           (bpb->BPB_NumFATs == 2) &&
           (bpb->BPB_Media != 0);
};

bool fat16_validate_ext_header(const FAT16ExtendedHeader *ext_header)
{
    return ext_header->signature == 0x29 && ext_header->drive_number == 0x80;
};

static void fat16_dump_ext_header(const FAT16ExtendedHeader *ext_header, const char *msg)
{
    uint8_t volume_label[12] = {};
    uint8_t system_ident[9] = {};
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("Physical Drive Number: 0x%x\n", ext_header->drive_number);
    kprintf("Reserved: 0x%x\n", ext_header->nt_flags);
    kprintf("Extended Boot Signature: %d\n", ext_header->signature);
    kprintf("Volume ID: 0x%x\n", ext_header->volume_id);
    mcpy(volume_label, ext_header->volume_label, 11);
    mcpy(system_ident, ext_header->system_ident, 7);
    kprintf("Volume Name of Partition: %s\n", volume_label);
    kprintf("FAT Name: %s\n", system_ident);
    kprintf("----------------------------------\n");
    return;
};

static void fat16_dump_base_header(const BIOSParameterBlock *bpb, const char *msg)
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
    return;
};

static uint32_t calculate_root_dir_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t root_directory_offset = bpb->BPB_BytsPerSec * (bpb->BPB_RsvdSecCnt + bpb->BPB_NumFATs * bpb->BPB_FATSz16);
    return root_directory_offset;
};

static uint32_t calculate_root_dir_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)

{
    const uint32_t root_directory_absolute = partition_offset + calculate_root_dir_offset(bpb);
    return root_directory_absolute;
};

uint32_t calculate_fat_table_offset(const BIOSParameterBlock *bpb, const uint32_t partition_offset)
{
    const uint32_t fat_table_offset = partition_offset + bpb->BPB_RsvdSecCnt * bpb->BPB_BytsPerSec;
    return fat_table_offset;
};

int is_first_lfn_entry(const FAT16LongDirectoryEntry *lfn_entry)
{
    return (lfn_entry->ldir_ord & 0x40) == 0;
};

int is_last_lfn_entry(const FAT16LongDirectoryEntry *lfn_entry)
{
    return (lfn_entry->ldir_ord & 0x40) != 0;
};

int is_lfn_entry(uint8_t *entry)
{
    if (entry[11] == 0x0F)
    {
        return 1;
    };
    return 0;
}

int is_sfn_entry(uint8_t *entry)
{
    if (entry[11] != 0x0F && entry[0] != 0xE5)
    {
        return 1;
    };
    return 0;
};

static void fat16_dump_root_dir_entries(const BIOSParameterBlock *header, Stream *stream)
{
    const uint32_t root_dir_size = header->BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_entries = root_dir_size / sizeof(FAT16DirectoryEntry);

    for (int i = 0, file = 1; i < root_dir_entries; i++)
    {
        FAT16DirectoryEntry entry = {};
        stream_read(stream, (uint8_t *)&entry, sizeof(FAT16DirectoryEntry));

        if (is_sfn_entry((uint8_t *)&entry))
        {
            if (entry.file_size == 0)
            {
                continue;
            };
            FAT16TimeInfo create_time = fat16_convert_time(entry.create_time);
            FAT16DateInfo create_date = fat16_convert_date(entry.create_date);
            FAT16DateInfo last_access_date = fat16_convert_date(entry.last_access_date);
            FAT16TimeInfo mod_time = fat16_convert_time(entry.modification_time);
            FAT16DateInfo mod_date = fat16_convert_date(entry.modification_date);

            kprintf("==========================\n");
            kprintf("=   File %d:\n", file);
            kprintf("==========================\n");
            kprintf("=   Filename: %s\n", entry.file_name);
            kprintf("=   Attributes: 0x%x\n", entry.attributes);
            kprintf("=   Creation Time: %d:%d:%d\n", create_time.hour, create_time.minute, create_time.second);
            kprintf("=   Creation Date: %d.%d.%d\n", create_date.day, create_date.month, create_date.year);
            kprintf("=   Last Access Date: %d.%d.%d\n", last_access_date.day, last_access_date.month, last_access_date.year);
            kprintf("=   High Cluster: %d\n", entry.high_cluster);
            kprintf("=   Modification Time: %d:%d:%d\n", mod_time.hour, mod_time.minute, mod_time.second);
            kprintf("=   Modification Date: %d.%d.%d\n", mod_date.day, mod_date.month, mod_date.year);
            kprintf("=   Low Cluster: %d\n", entry.low_cluster);
            kprintf("=   File Size: %d Bytes\n", entry.file_size);
            kprintf("==========================\n");
            file++;
        };

        if (is_lfn_entry((uint8_t *)&entry))
        {
            stream_read(stream, (uint8_t *)&entry, sizeof(FAT16DirectoryEntry));
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
        return -1;
    };

    if (!fat16_validate_header(&fat16_header.bpb))
    {
        kprintf("Error: Invalid FAT16 Header.\n");
        return -1;
    };

    if (!fat16_validate_ext_header(&fat16_header.ext))
    {
        kprintf("Error: Invalid FAT16 Extended Header.\n");
    };
    fat16_dump_base_header(&fat16_header.bpb, "");
    fat16_dump_ext_header(&fat16_header.ext, "");

    const uint32_t root_dir_offset = calculate_root_dir_offset(&fat16_header.bpb);
    const uint32_t root_dir_absolute = calculate_root_dir_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t root_dir_size = fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_entries = root_dir_size / sizeof(FAT16DirectoryEntry);

    kprintf("Root Directory Offset: 0x%x\n", root_dir_offset);
    kprintf("Root Directory Absolute: 0x%x\n", root_dir_absolute);
    kprintf("Root Directory Size: %d\n", root_dir_size);
    kprintf("Root Directory Entries: %d\n", root_dir_entries);

    Stream root_dir = {};
    stream_init(&root_dir, disk);
    stream_seek(&root_dir, root_dir_absolute);
    fat16_dump_root_dir_entries(&fat16_header.bpb, &root_dir);

    const uint32_t fat_table_offset = calculate_fat_table_offset(&fat16_header.bpb, partition_offset);
    const uint32_t fat_table_size_bytes = fat16_header.bpb.BPB_FATSz16 * fat16_header.bpb.BPB_BytsPerSec;
    const uint32_t fat_table_entries = fat_table_size_bytes / sizeof(uint16_t);
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    if (mode != VNODE_MODE_READ)
    {
        return 0x0;
    };
    return 0x0;
};