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
    .read_cb = 0x0,
    .name = "FAT16",
};

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

typedef struct ExtendedBIOSParameterBlock
{
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
} __attribute__((packed)) ExtendedBIOSParameterBlock;

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

typedef enum FAT16EntryType
{
    FAT16_ENTRY_TYPE_FILE,
    FAT16_ENTRY_TYPE_DIRECTORY,
} FAT16EntryType;

typedef struct FAT16Directory
{
    struct FAT16DirectoryEntry *entry;
    uint32_t total;
    uint32_t start_sector;
    uint32_t end_sector;
} FAT16Directory;

typedef struct FAT16Entry
{
    union
    {
        FAT16DirectoryEntry *file; // Pointer to the directory entry of the file
        FAT16Directory *dir;       // Pointer to the directory entry of the directory
    };
    FAT16EntryType type;
} FAT16Entry;

typedef struct FAT16FileDescriptor
{
    FAT16Entry *entry;
    uint32_t pos;
} FAT16FileDescriptor;

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
    fat16.read_cb = fat16_read;
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

static uint32_t calculate_root_dir_area_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t root_directory_offset = bpb->BPB_BytsPerSec * (bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz16));
    return root_directory_offset;
};

static uint32_t calculate_root_dir_area_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)

{
    const uint32_t root_dir_area_absolute = partition_offset + calculate_root_dir_area_offset(bpb);
    return root_dir_area_absolute;
};

static uint32_t calculate_fat_area_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t fat_area_offset = bpb->BPB_RsvdSecCnt * bpb->BPB_BytsPerSec;
    return fat_area_offset;
};

static uint32_t calculate_fat_area_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)
{
    const uint32_t fat_area_absolute = partition_offset + calculate_fat_area_offset(bpb);
    return fat_area_absolute;
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

int fat16_resolve(ATADev *dev)
{
    Stream header_stream;
    const uint32_t partition_offset = 0x100000;
    stream_init(&header_stream, dev);
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
    fat16_dump_base_header(&fat16_header.bpb, "", 0);
    fat16_dump_ebpb_header(&fat16_header.ebpb, "", 0);
    const uint32_t root_dir_area_offset = calculate_root_dir_area_offset(&fat16_header.bpb);
    const uint32_t root_dir_area_absolute = calculate_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t root_dir_area_size = fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    const uint32_t root_dir_area_entries = root_dir_area_size / sizeof(FAT16DirectoryEntry);

    kprintf("Root Dir Area Offset: 0x%x\n", root_dir_area_offset);
    kprintf("Root Dir Area Absolute: 0x%x\n", root_dir_area_absolute);
    kprintf("Root Dir Area Size: %d\n", root_dir_area_size);
    kprintf("Root Dir Area Entries: %d\n", root_dir_area_entries);

    Stream root_dir = {};
    stream_init(&root_dir, dev);
    stream_seek(&root_dir, root_dir_area_absolute);
    // fat16_dump_root_dir_entries(&fat16_header.bpb, &root_dir);

    const uint32_t fat_area_offset = calculate_fat_area_offset(&fat16_header.bpb);
    const uint32_t fat_area_absolute = calculate_fat_area_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t fat_area_size = fat16_header.bpb.BPB_FATSz16 * fat16_header.bpb.BPB_BytsPerSec;
    const uint32_t fat_area_entries = fat_area_size / sizeof(uint16_t);

    kprintf("FAT Area Offset: 0x%x\n", fat_area_offset);
    kprintf("FAT Area Absolute: 0x%x\n", fat_area_absolute);
    kprintf("FAT Area Size: %d\n", fat_area_size);
    kprintf("FAT Area Entries: %d\n", fat_area_entries);

    const uint32_t total_sectors = fat16_header.bpb.BPB_TotSec16 == 0 ? fat16_header.bpb.BPB_TotSec32 : fat16_header.bpb.BPB_TotSec16;
    kprintf("Total Sectors: %d\n", total_sectors);
    const uint32_t root_dir_sectors = (fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry) + fat16_header.bpb.BPB_BytsPerSec - 1) / fat16_header.bpb.BPB_BytsPerSec;
    kprintf("Root Dir Sectors: %d\n", root_dir_sectors);
    const uint32_t first_data_sector = fat16_header.bpb.BPB_RsvdSecCnt + fat16_header.bpb.BPB_NumFATs * fat16_header.bpb.BPB_FATSz16 + (root_dir_sectors);
    kprintf("First Data Sector: %d\n", first_data_sector);
    const uint32_t first_fat_sector = fat16_header.bpb.BPB_RsvdSecCnt;
    kprintf("First FAT Sector: %d\n", first_fat_sector);
    const uint32_t data_sectors = total_sectors - (fat16_header.bpb.BPB_RsvdSecCnt + (fat16_header.bpb.BPB_NumFATs * fat16_header.bpb.BPB_FATSz16 + root_dir_sectors));
    kprintf("Data Sectors: %d\n", data_sectors);
    const uint32_t total_clusters = data_sectors / fat16_header.bpb.BPB_SecPerClus;
    kprintf("Total Clusters: %d\n", total_clusters);

    if (total_clusters > FAT16_MAX_CLUSTERS || total_clusters < FAT16_MIN_CLUSTERS)
    {
        kprintf("Error: Invalid FAT16 Header.\n");
        return -EIO;
    };
    return 0;
};

void fat16_userland_filename_to_native(uint8_t *native_file_name, uint8_t *file_name)
{
    uint8_t i, j;

    for (i = 0; i < 11; i++)
    {
        native_file_name[i] = ' ';
    };

    for (i = 0; i < 8 && file_name[i] != '\0' && file_name[i] != '.'; i++)
    {
        native_file_name[i] = file_name[i];
    };

    if (file_name[i] == '.')
    {
        for (j = 0, i++; j < 3 && file_name[i] != '\0'; i++, j++)
        {
            native_file_name[8 + j] = file_name[i];
        };
    };
    return;
};

static uint32_t fat16_combine_cluster(const uint16_t high_cluster, const uint16_t low_cluster)
{
    return ((uint32_t)high_cluster << 16) | low_cluster;
};

FAT16Entry *fat16_get_entry(ATADev *dev, PathNode *path_identifier)
{
    const uint32_t partition_offset = 0x100000;
    const uint32_t root_dir_area_absolute = calculate_root_dir_area_absolute(&fat16_header.bpb, partition_offset);

    Stream stream = {};
    stream_init(&stream, dev);
    stream_seek(&stream, root_dir_area_absolute);

    const uint32_t root_dir_sectors = (fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry) + fat16_header.bpb.BPB_BytsPerSec - 1) / fat16_header.bpb.BPB_BytsPerSec;
    const uint32_t first_data_sector = fat16_header.bpb.BPB_RsvdSecCnt + fat16_header.bpb.BPB_NumFATs * fat16_header.bpb.BPB_FATSz16 + (root_dir_sectors);
    const uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
    kprintf("First Root Dir Sector: %d\n", first_root_dir_sector);
    const uint32_t root_dir_size = fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry);
    kprintf("Root Dir Size: %d\n", root_dir_size);
    uint8_t buffer[root_dir_size];

    const int32_t res = stream_read(&stream, buffer, root_dir_size);

    if (res < 0)
    {
        kprintf("StreamError: An error occurred while reading FAT16 Root Directory Entries.\n");
        return 0x0;
    };

    while (path_identifier)
    {
        FAT16DirectoryEntry *curr_root_entry = (FAT16DirectoryEntry *)buffer;

        for (int i = 0; i < fat16_header.bpb.BPB_RootEntCnt; i++, curr_root_entry++)
        {
            if (curr_root_entry->file_name[0] != 0x0)
            {
                uint8_t native_file_name[11] = {};
                fat16_userland_filename_to_native(native_file_name, (uint8_t *)path_identifier);

                if (mcmp(curr_root_entry->file_name, native_file_name, 11) == 0)
                {
                    FAT16Entry *fat16_entry = kcalloc(sizeof(FAT16Entry));

                    if (!fat16_entry)
                    {
                        kprintf("FAT16 Error: Not enough memory. Cannot allocate memory for a FAT16 entry.\n");
                        return 0x0;
                    };
                    fat16_entry->type =
                        curr_root_entry->attributes & DIRECTORY
                            ? FAT16_ENTRY_TYPE_DIRECTORY
                            : FAT16_ENTRY_TYPE_FILE;

                    FAT16DirectoryEntry *fat16_dir_entry = kcalloc(sizeof(FAT16DirectoryEntry));
                    mcpy(fat16_dir_entry, curr_root_entry, sizeof(FAT16DirectoryEntry));

                    fat16_entry->file = fat16_dir_entry;
                    kprintf("==========================\n");
                    kprintf("=   FAT16Entry:\n");
                    kprintf("==========================\n");
                    kprintf("=   Filename: %s\n", fat16_entry->file->file_name);
                    kprintf("=   Attribute: 0x%x\n", fat16_entry->file->attributes);
                    kprintf("=   Cluster: %d\n", fat16_combine_cluster(fat16_entry->file->high_cluster, fat16_entry->file->low_cluster));
                    kprintf("=   Type: %s\n", fat16_entry->file->attributes & DIRECTORY ? "Directory" : "File");
                    kprintf("==========================\n");
                    return fat16_entry;
                };
            };
        };
        path_identifier = path_identifier->next;
    };
    return 0x0;
};

void *fat16_open(ATADev *dev, PathNode *path, VNODE_MODE mode)
{
    if (mode != V_READ)
    {
        kprintf("FAT16 Error: Only read mode is supported.\n");
        return 0x0;
    };

    if (dev == 0x0 || dev->fs == 0x0 || path == 0x0)
    {
        kprintf("FAT16 Error: Device, Filesystem or path not initialized.\n");
        return 0x0;
    };
    FAT16FileDescriptor *fd = kcalloc(sizeof(FAT16FileDescriptor));

    if (fd == 0x0)
    {
        kprintf("FAT16 Error: Memory allocation failed.\n");
        kfree(fd);
        return 0x0;
    };
    FAT16Entry *entry = fat16_get_entry(dev, path);

    if (!entry)
    {
        kfree(fd);
        kprintf("FAT16: FATEntry not found.\n");
        return 0x0;
    };
    fd->entry = entry;
    fd->pos = 0;
    return fd;
};

uint16_t fat16_read_fat_entry(Stream *stream, uint32_t cluster)
{
    uint8_t fat_entry[2];
    // Calculate the sector containing the FAT entry for the given cluster
    uint32_t fat_sector = fat16_header.bpb.BPB_RsvdSecCnt + (cluster * 2) / fat16_header.bpb.BPB_BytsPerSec;
    // Calculate the offset within the sector for the FAT entry
    uint32_t fat_offset = (cluster * 2) % fat16_header.bpb.BPB_BytsPerSec;
    // Seek to the appropriate position in the FAT
    uint32_t fat_entry_position = fat_sector * fat16_header.bpb.BPB_BytsPerSec + fat_offset;
    stream_seek(stream, fat_entry_position);
    // Read the 16-bit FAT entry
    stream_read(stream, fat_entry, sizeof(uint16_t));
    return *(uint16_t *)fat_entry;
};

uint32_t fat16_cluster_to_sector(uint32_t cluster)
{
    const uint32_t root_dir_sectors = (fat16_header.bpb.BPB_RootEntCnt * sizeof(FAT16DirectoryEntry) + fat16_header.bpb.BPB_BytsPerSec - 1) / fat16_header.bpb.BPB_BytsPerSec;
    const uint32_t data_start_sector = fat16_header.bpb.BPB_RsvdSecCnt + (fat16_header.bpb.BPB_NumFATs * fat16_header.bpb.BPB_FATSz16) + root_dir_sectors;
    return data_start_sector + ((cluster - 2) * fat16_header.bpb.BPB_SecPerClus);
};

size_t fat16_read(ATADev *dev, void *descriptor, uint8_t *buffer, size_t n_bytes, size_t n_blocks)
{
    const uint32_t partition_offset = 0x100000;

    Stream fat_stream = {}, data_stream = {};
    stream_init(&fat_stream, dev);
    stream_init(&data_stream, dev);

    FAT16FileDescriptor *fat16_descriptor = descriptor;
    FAT16Entry *fat16_entry = fat16_descriptor->entry;
    kprintf("fat16_read - fat16_entry->file: %s\n", fat16_entry->file);
    uint32_t offset = fat16_descriptor->pos;

    for (size_t i = 0; i < n_blocks; ++i)
    {
        const uint32_t cluster = fat16_combine_cluster(fat16_descriptor->entry->file->high_cluster, fat16_descriptor->entry->file->low_cluster);
        const uint32_t sector = fat16_cluster_to_sector(cluster);
        const uint32_t offset_within_cluster = offset % (fat16_header.bpb.BPB_SecPerClus * dev->sector_size);
        const uint32_t data_position = partition_offset + (sector * dev->sector_size) + offset_within_cluster;
        // Read the data
        stream_seek(&data_stream, data_position);
        stream_read(&data_stream, buffer, n_bytes);
        // Update buffer and offset for the next iteration
        buffer += n_bytes;
        offset += n_bytes;
    };
    fat16_descriptor->pos = offset;
    return (size_t)(offset - fat16_descriptor->pos);
};