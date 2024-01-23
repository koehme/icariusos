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

Superblock *fat16_init(void)
{
    fat16.resolve_cb = fat16_resolve;
    fat16.open_cb = fat16_open;
    return &fat16;
};

bool fat16_validate_header(const FAT16BaseHeader *header)
{
    return (header->jmp_short[0] == 0xEB && header->jmp_short[2] == 0x90) &&
           (header->bytes_per_sector == 512) &&
           (header->sectors_per_cluster >= 1) &&
           (header->fat_table_copies == 2) &&
           (header->media_desc_type != 0);
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

static void fat16_dump_base_header(const FAT16BaseHeader *header, const char *msg)
{
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("Jump Code + NOP: 0x%x 0x%x\n", header->jmp_short[0], header->jmp_short[1], header->jmp_short[2]);
    kprintf("OEM Name: %s\n", header->oem_ident);
    kprintf("Bytes Per Sector: %d\n", header->bytes_per_sector);
    kprintf("Sectors Per Cluster: %d\n", header->sectors_per_cluster);
    kprintf("Reserved Sectors: %d\n", header->reserved_sectors);
    kprintf("Number of Copies of FAT: %d\n", header->fat_table_copies);
    kprintf("Maximum Root Directory Entries: %d\n", header->root_directories);
    kprintf("Number of Sectors: %d\n", header->total_sectors);
    kprintf("Media Descriptor: 0x%x\n", header->media_desc_type);
    kprintf("Sectors Per FAT: %d\n", header->sectors_per_fat);
    kprintf("Sectors Per Track: %d\n", header->sectors_per_track);
    kprintf("Number of Heads: %d\n", header->heads);
    kprintf("Number of Hidden Sectors in Partition: %d\n", header->hidden_sectors);
    kprintf("Number of Sectors in Partition: %d\n", header->large_total_sectors);
    kprintf("----------------------------------\n");
    return;
};

static uint32_t calculate_root_directory_offset(const FAT16BaseHeader *header)
{
    const uint32_t root_directory_offset = header->bytes_per_sector * (header->reserved_sectors + header->fat_table_copies * header->sectors_per_fat);
    return root_directory_offset;
};

static uint32_t calculate_root_directory_absolute(const FAT16BaseHeader *header, const uint32_t partition_offset)

{
    const uint32_t root_directory_absolute = partition_offset + calculate_root_directory_offset(header);
    return root_directory_absolute;
};

uint32_t calculate_fat_table_offset(const FAT16BaseHeader *header, const uint32_t partition_offset)
{
    const uint32_t fat_table_offset = partition_offset + header->reserved_sectors * header->bytes_per_sector;
    return fat_table_offset;
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

    if (!fat16_validate_header(&fat16_header.base))
    {
        kprintf("Error: Invalid FAT16 Header.\n");
        return -1;
    };

    if (!fat16_validate_ext_header(&fat16_header.ext))
    {
        kprintf("Error: Invalid FAT16 Extended Header.\n");
    };
    fat16_dump_base_header(&fat16_header.base, "");
    fat16_dump_ext_header(&fat16_header.ext, "");

    const uint32_t root_directory_offset = calculate_root_directory_offset(&fat16_header.base);
    const uint32_t root_directory_absolute = calculate_root_directory_absolute(&fat16_header.base, partition_offset);
    const uint32_t root_directory_size = fat16_header.base.root_directories * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_entries = root_directory_size / sizeof(FAT16DirectoryEntry);

    kprintf("Root Directory Offset: 0x%x\n", root_directory_offset);
    kprintf("Root Directory Absolute: 0x%x\n", root_directory_absolute);
    kprintf("Root Directory Size: %d\n", root_directory_size);
    kprintf("Root Directory Entries: %d\n", root_dir_entries);

    const uint32_t fat_table_offset = calculate_fat_table_offset(&fat16_header.base, partition_offset);
    const uint32_t fat_table_size_bytes = fat16_header.base.sectors_per_fat * fat16_header.base.bytes_per_sector;
    const uint32_t fat_table_entries = fat_table_size_bytes / sizeof(FAT16Entry);
    kprintf("FAT Table Offset: 0x%x\n", fat_table_offset);
    kprintf("FAT Table Size: %d\n", fat_table_size_bytes);
    kprintf("FAT Table Entries: %d\n", fat_table_entries);
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    return 0x0;
};