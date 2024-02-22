/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16.h"
#include "stream.h"
#include "mem.h"

FileSystem fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .read_cb = 0x0,
    .close_cb = 0x0,
    .name = "FAT16",
};

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

typedef struct BIOSParameterBlock
{
    uint8_t jmp[3];        // jump over the disk format information (the BPB and EBPB)
    uint8_t oem[8];        // oem identifier. The first 8 Bytes (3 - 10) is the version of DOS being used
    uint16_t byts_per_sec; // number of Bytes per sector (remember, all numbers are in the little-endian format)
    uint8_t sec_per_clus;  // number of sectors per cluster
    uint16_t rsvd_sec;     // number of reserved sectors. The boot record sectors are included in this value
    uint8_t num_fats;      // number of File Allocation Tables (FAT's) on the storage media
    uint16_t root_ent_cnt; // number of root directory entries (must be set so that the root directory occupies entire sectors)
    uint16_t tot_sec_16;   // total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20
    uint8_t media;         // indicates the media descriptor type
    uint16_t fatsz16;      // number of sectors per FAT
    uint16_t sec_per_trk;  // number of sectors per track
    uint16_t num_heads;    // number of heads or sides on the storage media
    uint32_t hidd_sec;     // number of hidden sectors. (i.e. the LBA of the beginning of the partition
    uint32_t tot_sec_32;   // large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not fit in the Number of Sectors entry at 0x13
} __attribute__((packed)) BIOSParameterBlock;

typedef struct ExtendedBIOSParameterBlock
{
    uint8_t drv_num;
    uint8_t reserved1;
    uint8_t boot_sig;
    uint32_t vol_id;
    uint8_t vol_lab[11];
    uint8_t fil_sys_type[8];
} __attribute__((packed)) ExtendedBIOSParameterBlock;

typedef struct FAT16DirEntry
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
} __attribute__((packed)) FAT16DirEntry;

typedef struct FAT16InternalHeader
{
    BIOSParameterBlock bpb;
    ExtendedBIOSParameterBlock ebpb;
} __attribute__((packed)) FAT16InternalHeader;

typedef struct FAT16TimeInfo
{
    int32_t hour;
    int32_t minute;
    int32_t second;
} FAT16TimeInfo;

typedef struct FAT16DateInfo
{
    int32_t day;
    int32_t month;
    int32_t year;
} FAT16DateInfo;

typedef enum FAT16EntryType
{
    FAT16_ENTRY_TYPE_FILE,
    FAT16_ENTRY_TYPE_DIRECTORY,
} FAT16EntryType;

typedef struct FAT16Folder
{
    struct FAT16DirEntry *entry;
    uint32_t total;
    uint32_t start_sector;
    uint32_t end_sector;
} FAT16Folder;

typedef struct FAT16Entry
{
    union
    {
        FAT16DirEntry *file; // Pointer to the file
        FAT16Folder *dir;    // Pointer to the folder
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
        .jmp = {0},
        .oem = {0},
        .byts_per_sec = 0,
        .sec_per_clus = 0,
        .rsvd_sec = 0,
        .num_fats = 0,
        .root_ent_cnt = 0,
        .tot_sec_16 = 0,
        .media = 0,
        .fatsz16 = 0,
        .sec_per_trk = 0,
        .num_heads = 0,
        .hidd_sec = 0,
        .tot_sec_32 = 0,
    },
    .ebpb = {
        .drv_num = 0,
        .reserved1 = 0,
        .boot_sig = 0,
        .vol_id = 0,
        .vol_lab = {0},
        .fil_sys_type = {0},
    },
};

static uint32_t convert_data_cluster_to_sector(const uint32_t cluster)
{
    const uint32_t root_dir_sectors = (fat16_header.bpb.root_ent_cnt * sizeof(FAT16DirEntry) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
    const uint32_t data_start_sector = fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16) + root_dir_sectors;
    return data_start_sector + ((cluster - 2) * fat16_header.bpb.sec_per_clus);
};

static uint16_t read_next_cluster(Stream *fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster)
{
    uint8_t fat_entry[2] = {};
    const uint32_t fat_entry_offset = partition_offset + fat16_header.bpb.rsvd_sec * fat16_header.bpb.byts_per_sec + curr_cluster * 2;
    stream_seek(fat_stream, fat_entry_offset);
    stream_read(fat_stream, fat_entry, 2 * sizeof(uint8_t));
    return ((uint16_t)fat_entry[0]) | ((uint16_t)fat_entry[1] << 8);
};

static FAT16TimeInfo convert_time(const uint16_t time)
{
    FAT16TimeInfo time_info = {};
    time_info.second = (time & 0x1F) * 2;
    time_info.minute = (time >> 5) & 0x3F;
    time_info.hour = (time >> 11) & 0x1F;
    return time_info;
};

static FAT16DateInfo convert_date(const uint16_t date)
{
    FAT16DateInfo date_info = {};
    date_info.day = date & 0x1F;
    date_info.month = (date >> 5) & 0x0F;
    const uint16_t original_year = (date >> 9) & 0x7F;
    date_info.year = (original_year == 0) ? 0 : (original_year + 1980);
    return date_info;
};

FileSystem *fat16_init(void)
{
    fat16.resolve_cb = fat16_resolve;
    fat16.open_cb = fat16_open;
    fat16.read_cb = fat16_read;
    fat16.close_cb = fat16_close;
    return &fat16;
};

static bool fat16_validate_header(const FAT16InternalHeader *header)
{
    const bool has_signature = (header->bpb.jmp[0] == 0xEB && header->bpb.jmp[2] == 0x90);
    const bool has_header = has_signature &&
                            (header->bpb.byts_per_sec == 512) &&
                            (header->bpb.sec_per_clus >= 1) &&
                            (header->bpb.num_fats == 2) &&
                            (header->bpb.media != 0);

    if (!has_header)
    {
        return false;
    };
    const bool has_ebpb = (header->ebpb.boot_sig == 0x29) && (header->ebpb.drv_num == 0x80);

    if (has_ebpb)
    {
        const uint32_t total_sectors = (header->bpb.tot_sec_32 != 0) ? header->bpb.tot_sec_32 : header->bpb.tot_sec_16;
        const uint32_t disk_size = total_sectors * header->bpb.byts_per_sec;

        if (total_sectors > disk_size)
        {
            return false;
        };
    };
    return true;
};

static void fat16_dump_ebpb_header(const ExtendedBIOSParameterBlock *ebpb, const char *msg, const int32_t delay)
{
    uint8_t vol_lab[12] = {};
    uint8_t fil_sys_type[9] = {};
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("drv_num: 0x%x\n", ebpb->drv_num);
    kprintf("reserved1: 0x%x\n", ebpb->reserved1);
    kprintf("boot_sig: %d\n", ebpb->boot_sig);
    kprintf("vol_id: 0x%x\n", ebpb->vol_id);
    mcpy(vol_lab, ebpb->vol_lab, 11);
    mcpy(fil_sys_type, ebpb->fil_sys_type, 7);
    kprintf("vol_lab: %s\n", vol_lab);
    kprintf("fil_sys_type: %s\n", fil_sys_type);
    kprintf("----------------------------------\n");
    kdelay(delay);
    return;
};

static void fat16_dump_base_header(const BIOSParameterBlock *bpb, const char *msg, const int32_t delay)
{
    kprintf(msg);
    kprintf("----------------------------------\n");
    kprintf("jmp: 0x%x 0x%x\n", bpb->jmp[0], bpb->jmp[1], bpb->jmp[2]);
    kprintf("oem: %s\n", bpb->oem);
    kprintf("byts_per_sec: %d\n", bpb->byts_per_sec);
    kprintf("sec_per_clus: %d\n", bpb->sec_per_clus);
    kprintf("rsvd_sec: %d\n", bpb->rsvd_sec);
    kprintf("num_fats: %d\n", bpb->num_fats);
    kprintf("root_ent_cnt: %d\n", bpb->root_ent_cnt);
    kprintf("tot_sec_16: %d\n", bpb->tot_sec_16);
    kprintf("media: 0x%x\n", bpb->media);
    kprintf("fatsz16: %d\n", bpb->fatsz16);
    kprintf("sec_per_trk: %d\n", bpb->sec_per_trk);
    kprintf("num_heads: %d\n", bpb->num_heads);
    kprintf("hidd_sec: %d\n", bpb->hidd_sec);
    kprintf("tot_sec_32: %d\n", bpb->tot_sec_32);
    kprintf("----------------------------------\n");
    kdelay(delay);
    return;
};

static uint32_t calc_root_dir_area_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t root_directory_offset = bpb->byts_per_sec * (bpb->rsvd_sec + (bpb->num_fats * bpb->fatsz16));
    return root_directory_offset;
};

static uint32_t calc_root_dir_area_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)

{
    const uint32_t root_dir_area_absolute = partition_offset + calc_root_dir_area_offset(bpb);
    return root_dir_area_absolute;
};

static uint32_t calc_fat_area_offset(const BIOSParameterBlock *bpb)
{
    const uint32_t fat_area_offset = bpb->rsvd_sec * bpb->byts_per_sec;
    return fat_area_offset;
};

static uint32_t calc_fat_area_absolute(const BIOSParameterBlock *bpb, const uint32_t partition_offset)
{
    const uint32_t fat_area_absolute = partition_offset + calc_fat_area_offset(bpb);
    return fat_area_absolute;
};

static void print_dir_entry(size_t i, FAT16DirEntry *entry, const int32_t delay)
{
    if (entry->file_name[0] == 0x00)
    {
        return;
    };
    const FAT16TimeInfo create_time = convert_time(entry->create_time);
    const FAT16DateInfo create_date = convert_date(entry->create_date);
    const FAT16DateInfo last_access_date = convert_date(entry->last_access_date);
    const FAT16TimeInfo mod_time = convert_time(entry->modification_time);
    const FAT16DateInfo mod_date = convert_date(entry->modification_date);

    uint8_t buffer[12] = {};
    mcpy(buffer, entry->file_name, 11);

    kprintf("==========================\n");
    kprintf("=   RootDirEntry %d:\n", i);
    kprintf("==========================\n");
    kprintf("=   Filename: %s\n", buffer);
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

static void print_lfn_entry(size_t i, FAT16DirEntry *entry, const int32_t delay)
{
    if (entry->file_name[0] == 0x00)
    {
        return;
    };
    kprintf("==========================\n");
    kprintf("=   RootDirLFNEntry %d:\n", i);
    kprintf("==========================\n");
    kprintf("=   LFN: %s (Long File Name NOT SUPPORTED)\n", "-");
    kprintf("==========================\n");
    kdelay(delay);
    return;
};

static void dump_root_dir_entries(const BIOSParameterBlock *bpb, Stream *stream)
{
    const uint32_t root_dir_size = bpb->root_ent_cnt * sizeof(FAT16DirEntry);
    const uint32_t root_dir_entries = root_dir_size / sizeof(FAT16DirEntry);

    for (size_t i = 0; i < bpb->root_ent_cnt; i++)
    {
        FAT16DirEntry entry = {};
        stream_read(stream, (uint8_t *)&entry, sizeof(FAT16DirEntry));

        if (entry.attributes == LFN)
        {
            print_lfn_entry(i, &entry, FAT16_DEBUG_DELAY);
            continue;
        };
        print_dir_entry(i, &entry, FAT16_DEBUG_DELAY);
    };
    return;
};

int32_t fat16_resolve(ATADev *dev)
{
    Stream header_stream = {};
    const uint32_t partition_offset = 0x100000;
    stream_init(&header_stream, dev);
    stream_seek(&header_stream, partition_offset);
    const int32_t res = stream_read(&header_stream, (uint8_t *)&fat16_header, sizeof(FAT16InternalHeader));

    if (res != 0)
    {
        kprintf("FAT16 Error: Failed to read FAT16 Header\n");
        return -EIO;
    };

    if (!fat16_validate_header(&fat16_header))
    {
        kprintf("FAT16 Error: Invalid FAT16 Header\n");
        return -EIO;
    };
    fat16_dump_base_header(&fat16_header.bpb, "", 0);
    fat16_dump_ebpb_header(&fat16_header.ebpb, "", 0);
    const uint32_t root_dir_area_offset = calc_root_dir_area_offset(&fat16_header.bpb);
    const uint32_t root_dir_area_absolute = calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t root_dir_area_size = fat16_header.bpb.root_ent_cnt * sizeof(FAT16DirEntry);
    const uint32_t root_dir_area_entries = root_dir_area_size / sizeof(FAT16DirEntry);

    kprintf("Root Dir Area Offset: 0x%x\n", root_dir_area_offset);
    kprintf("Root Dir Area Absolute: 0x%x\n", root_dir_area_absolute);
    kprintf("Root Dir Area Size: %d\n", root_dir_area_size);
    kprintf("Root Dir Area Entries: %d\n", root_dir_area_entries);

    Stream root_dir = {};
    stream_init(&root_dir, dev);
    stream_seek(&root_dir, root_dir_area_absolute);
    dump_root_dir_entries(&fat16_header.bpb, &root_dir);

    const uint32_t fat_area_offset = calc_fat_area_offset(&fat16_header.bpb);
    const uint32_t fat_area_absolute = calc_fat_area_absolute(&fat16_header.bpb, partition_offset);
    const uint32_t fat_area_size = fat16_header.bpb.fatsz16 * fat16_header.bpb.byts_per_sec;
    const uint32_t fat_area_entries = fat_area_size / sizeof(uint16_t);

    kprintf("FAT Area Offset: 0x%x\n", fat_area_offset);
    kprintf("FAT Area Absolute: 0x%x\n", fat_area_absolute);
    kprintf("FAT Area Size: %d\n", fat_area_size);
    kprintf("FAT Area Entries: %d\n", fat_area_entries);
    kdelay(FAT16_DEBUG_DELAY);

    const uint32_t total_sectors = fat16_header.bpb.tot_sec_16 == 0 ? fat16_header.bpb.tot_sec_32 : fat16_header.bpb.tot_sec_16;
    kprintf("Total Sectors: %d\n", total_sectors);
    const uint32_t root_dir_sectors = (fat16_header.bpb.root_ent_cnt * sizeof(FAT16DirEntry) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
    kprintf("Root Dir Sectors: %d\n", root_dir_sectors);
    const uint32_t first_data_sector = fat16_header.bpb.rsvd_sec + fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + (root_dir_sectors);
    kprintf("First Data Sector: %d\n", first_data_sector);
    const uint32_t first_fat_sector = fat16_header.bpb.rsvd_sec;
    kprintf("First FAT Sector: %d\n", first_fat_sector);
    const uint32_t data_sectors = total_sectors - (fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + root_dir_sectors));
    kprintf("Data Sectors: %d\n", data_sectors);
    const uint32_t total_clusters = data_sectors / fat16_header.bpb.sec_per_clus;
    kprintf("Total Clusters: %d\n", total_clusters);

    if (total_clusters > FAT16_MAX_CLUSTERS || total_clusters < FAT16_MIN_CLUSTERS)
    {
        kprintf("FAT16 Error: Invalid FAT16 Header\n");
        return -EIO;
    };
    return 0;
};

static void convert_userland_filename_to_native(uint8_t *native, const uint8_t *userland)
{
    uint8_t i, j = 0;

    for (i = 0; i < 11; i++)
    {
        native[i] = ' ';
    };

    for (i = 0; i < 8 && userland[i] != '\0' && userland[i] != '.'; i++)
    {
        native[i] = userland[i];
    };

    if (userland[i] == '.')
    {
        for (j = 0, i++; j < 3 && userland[i] != '\0'; i++, j++)
        {
            native[8 + j] = userland[i];
        };
    };
    return;
};

static uint32_t combine_clusters(const uint16_t high_cluster, const uint16_t low_cluster)
{
    const uint32_t cluster = (high_cluster << 16) | low_cluster;
    return cluster;
};

static FAT16DirEntry *get_entry_in_subdir(ATADev *dev, const uint16_t start_cluster, const uint8_t *native_file_name, FAT16Folder *fat16_folder, FAT16DirEntry *fat16_dir_entry)
{
    const uint32_t partition_offset = 0x100000;

    Stream fat_stream = {};
    Stream data_stream = {};
    stream_init(&fat_stream, dev);
    stream_init(&data_stream, dev);

    const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

    uint8_t buffer[max_cluster_size_bytes];
    uint16_t curr_cluster = start_cluster;

    const uint32_t num_entries = max_cluster_size_bytes / sizeof(FAT16DirEntry);

    while (curr_cluster <= FAT16_VALUE_END_OF_CHAIN)
    {
        const uint32_t sector = convert_data_cluster_to_sector(curr_cluster);
        const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
        stream_seek(&data_stream, data_pos);
        stream_read(&data_stream, buffer, max_cluster_size_bytes);

        uint8_t *curr_dir_entry = buffer;

        for (int i = 0; i < num_entries; i++)
        {
            char tmp[11] = {};
            mcpy(tmp, ((FAT16DirEntry *)curr_dir_entry)->file_name, 11);

            if (mcmp(tmp, native_file_name, 11) == 0)
            {
                const uint32_t start_sector = convert_data_cluster_to_sector(start_cluster);
                const uint32_t end_sector = convert_data_cluster_to_sector(curr_cluster);
                fat16_folder->start_sector = start_sector;
                fat16_folder->end_sector = end_sector;
                FAT16DirEntry *entry = (FAT16DirEntry *)curr_dir_entry;
                mcpy(fat16_dir_entry, curr_dir_entry, sizeof(FAT16DirEntry));
                return fat16_dir_entry;
            };
            curr_dir_entry += sizeof(FAT16DirEntry);
        };
        curr_cluster = read_next_cluster(&fat_stream, partition_offset, curr_cluster);
    };
    return 0x0;
};

static FAT16DirEntry *get_root_dir_entry(ATADev *dev, FAT16DirEntry *root_dir_entry, PathNode *path_identifier)
{
    const uint32_t partition_offset = 0x100000;
    const uint32_t root_dir_area_absolute = calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);

    Stream stream = {};
    stream_init(&stream, dev);
    stream_seek(&stream, root_dir_area_absolute);

    const uint32_t root_dir_sectors = (fat16_header.bpb.root_ent_cnt * sizeof(FAT16DirEntry) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
    const uint32_t first_data_sector = fat16_header.bpb.rsvd_sec + fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + (root_dir_sectors);
    const uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
    const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(FAT16DirEntry);
    uint8_t buffer[root_dir_size];

    const int32_t res = stream_read(&stream, buffer, root_dir_size);

    if (res < 0)
    {
        kprintf("StreamError: An error occurred while reading FAT16 Root Directory Entries\n");
        return 0x0;
    };
    FAT16DirEntry *curr_root_entry = (FAT16DirEntry *)buffer;

    for (size_t i = 0; i < fat16_header.bpb.root_ent_cnt; i++, curr_root_entry++)
    {
        if (curr_root_entry->file_name[0] != 0x0)
        {
            uint8_t native_dir_name[11] = {};
            convert_userland_filename_to_native(native_dir_name, (uint8_t *)path_identifier);

            if (mcmp(curr_root_entry->file_name, native_dir_name, 11) == 0)
            {
                mcpy(root_dir_entry, curr_root_entry, sizeof(FAT16DirEntry));
                break;
            };
        };
    };
    return root_dir_entry;
};

static FAT16Entry *get_entry(ATADev *dev, PathNode *path)
{
    FAT16DirEntry root_dir_entry = {};
    get_root_dir_entry(dev, &root_dir_entry, path);

    if (root_dir_entry.file_name[0] == 0x0)
    {
        kprintf("FAT16 Error: Root directory entry not found\n");
        return 0x0;
    };
    FAT16Entry *fat16_entry = kcalloc(sizeof(FAT16Entry));
    FAT16Folder *fat16_folder = kcalloc(sizeof(FAT16Folder));
    FAT16DirEntry *fat16_dir_entry = kcalloc(sizeof(FAT16DirEntry));
    fat16_entry->dir = fat16_folder;
    fat16_entry->dir->entry = fat16_dir_entry;

    if (path && path->next)
    {
        path = path->next;
    };
    uint32_t curr_cluster = combine_clusters(root_dir_entry.high_cluster, root_dir_entry.low_cluster);

    while (path)
    {
        uint8_t native[11] = {};
        convert_userland_filename_to_native(native, (uint8_t *)path);
        get_entry_in_subdir(dev, curr_cluster, native, fat16_folder, fat16_dir_entry);
        path = path->next;
        curr_cluster = combine_clusters(fat16_folder->entry->high_cluster, fat16_folder->entry->low_cluster);
    };
    fat16_entry->type = FAT16_ENTRY_TYPE_DIRECTORY;
    return fat16_entry;
};

void *fat16_open(ATADev *dev, PathNode *path, VNODE_MODE mode)
{
    if (mode != V_READ)
    {
        kprintf("FAT16 Error: Only read mode is supported\n");
        return 0x0;
    };

    if (dev == 0x0 || dev->fs == 0x0 || path == 0x0)
    {
        kprintf("FAT16 Error: Device, Filesystem or path not initialized\n");
        return 0x0;
    };
    FAT16FileDescriptor *fd = kcalloc(sizeof(FAT16FileDescriptor));

    if (fd == 0x0)
    {
        kprintf("FAT16 Error: Memory allocation failed\n");
        kfree(fd);
        return 0x0;
    };
    FAT16Entry *entry = get_entry(dev, path);

    if (!entry)
    {
        kfree(fd);
        kprintf("FAT16 Error: FATEntry not found\n");
        return 0x0;
    };
    fd->entry = entry;
    fd->pos = 0;
    return fd;
};

static uint32_t get_start_cluster_from_descriptor(FAT16FileDescriptor *fat16_descriptor)
{
    uint32_t start_cluster = 0;

    switch (fat16_descriptor->entry->type)
    {
    case FAT16_ENTRY_TYPE_DIRECTORY:
    {
        start_cluster = combine_clusters(fat16_descriptor->entry->dir->entry->high_cluster, fat16_descriptor->entry->dir->entry->low_cluster);
        break;
    };
    case FAT16_ENTRY_TYPE_FILE:
    {
        start_cluster = combine_clusters(fat16_descriptor->entry->file->high_cluster, fat16_descriptor->entry->file->low_cluster);
        break;
    };
    default:
    {
        break;
    };
    };
    return start_cluster;
};

// Read data from a FAT16 filesystem
size_t fat16_read(ATADev *dev, void *descriptor, uint8_t *buffer, const size_t n_bytes, const size_t n_blocks)
{
    // Define the offset of the partition
    const uint32_t partition_offset = 0x100000;
    // Essentially to seek through the streams
    Stream fat_stream = {}, data_stream = {};
    stream_init(&fat_stream, dev);
    stream_init(&data_stream, dev);
    // Cast the vfs internal back to the concrete fat16 descriptor
    FAT16FileDescriptor *fat16_descriptor = descriptor;
    // Maximum cluster size in bytes of a single FAT16 cluster
    const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
    // The given file descriptor holds a pointer to the root dir entry, which holds the fat16 start cluster number to read
    const uint32_t start_cluster = get_start_cluster_from_descriptor(fat16_descriptor);
    // Determine the starting cluster for reading
    uint16_t curr_cluster = (fat16_descriptor->pos / max_cluster_size_bytes) + start_cluster;
    const uint16_t first_cluster_offset = fat16_descriptor->pos % max_cluster_size_bytes;
    // Initialize bytes_read for tracking read progress and return to the  caller of the function the amount of readed bytes
    size_t bytes_read = 0;
    // Initialize remaining_bytes for tracking the termination of the following read loop
    size_t remaining_bytes = n_bytes * n_blocks - bytes_read;
    // Limit read_size to the smaller of remaining_bytes and the max cluster size in FAT16 filesystem
    size_t read_size = remaining_bytes < max_cluster_size_bytes ? remaining_bytes : max_cluster_size_bytes;

    while (remaining_bytes)
    {
        // Convert the logical fat16 cluster number to a physical sector
        const uint32_t sector = convert_data_cluster_to_sector(curr_cluster);
        // Use the previously calculated sector and add the partition offset and the first cluster to get the data position on die ata device
        const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec) + first_cluster_offset;
        // Seek to the data position and read data into the buffer
        stream_seek(&data_stream, data_pos);
        // Ensure that new data is appended to the buffer + bytes_read ensures the correct position so that previously written data is not overwritten
        const int32_t res = stream_read(&data_stream, buffer + bytes_read, read_size);

        if (res < 0)
        {
            kprintf("StreamError: An error occurred while reading FAT16\n");
            return bytes_read;
        };
        // Update read progress
        bytes_read += read_size;
        fat16_descriptor->pos += read_size;
        remaining_bytes -= read_size;
        // Determine the next cluster from the FAT table
        const uint16_t next_cluster = read_next_cluster(&fat_stream, partition_offset, curr_cluster);
        // Check if end of file is reached, if yes, exit loop
        if (next_cluster >= FAT16_VALUE_END_OF_CHAIN)
        {
            break;
        };
        // Update current cluster for the next iteration
        curr_cluster = next_cluster;
        // Adjust read_size based on remaining bytes and cluster size
        read_size = remaining_bytes < max_cluster_size_bytes ? remaining_bytes : max_cluster_size_bytes;
    };
    // Return the total bytes read
    return bytes_read;
};

int32_t fat16_close(void *internal)
{
    int32_t res = 0;
    FAT16FileDescriptor *fat16_descriptor = (FAT16FileDescriptor *)internal;

    if (fat16_descriptor == 0x0)
    {
        res = -EINVAL;
        return res;
    };
    kfree(fat16_descriptor->entry->dir->entry);
    kfree(fat16_descriptor->entry->dir);
    kfree(fat16_descriptor->entry);
    kfree(fat16_descriptor);
    return 0;
};