/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief FAT16 filesystem implementation.
 * @date 2024-11-13
 */

#include "fat16.h"
#include "stream.h"
#include "string.h"

/* FAT16 Values */
#define FAT16_VALUE_FREE 0x0000
#define FAT16_VALUE_RESERVED 0x0001
#define FAT16_VALUE_BAD_CLUSTER 0xFFF7
#define FAT16_VALUE_END_OF_CHAIN 0xFFF8
/* FAT16 Limits */
#define FAT16_MIN_CLUSTERS 4085
#define FAT16_MAX_CLUSTERS 65525
/* FAT16 Attributes */
#define READ_WRITE 0x00
#define READ_ONLY 0x01
#define HIDDEN 0x02
#define SYSTEM 0x04
#define VOLUME_ID 0x08
#define DIRECTORY 0x10
#define ARCHIVE 0x20
#define LFN (READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID)
#define DEVICE 0x40
#define DELETED 0xE5

typedef struct fat16 {
	resolve_fn resolve_cb;
	open_fn open_cb;
	read_fn read_cb;
	close_fn close_cb;
	stat_fn stat_cb;
	write_fn write_cb;
	char name[10];
} fat16_t;

typedef struct bpb {
	uint8_t jmp[3];	       // jump over the disk format information (the BPB and EBPB)
	uint8_t oem[8];	       // oem identifier. The first 8 Bytes (3 - 10) is the version of DOS being used
	uint16_t byts_per_sec; // number of Bytes per sector (remember, all numbers are in the little-endian format)
	uint8_t sec_per_clus;  // number of sectors per cluster
	uint16_t rsvd_sec;     // number of reserved sectors. The boot record sectors are included in this value
	uint8_t num_fats;      // number of File Allocation Tables (FAT's) on the storage media
	uint16_t root_ent_cnt; // number of root directory entries (must be set so that the root directory occupies entire sectors)
	uint16_t tot_sec_16;   // total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the
			       // actual count is stored in the Large Sector Count entry at 0x20
	uint8_t media;	       // indicates the media descriptor type
	uint16_t fatsz16;      // number of sectors per FAT
	uint16_t sec_per_trk;  // number of sectors per track
	uint16_t num_heads;    // number of heads or sides on the storage media
	uint32_t hidd_sec;     // number of hidden sectors. (i.e. the LBA of the beginning of the partition
	uint32_t tot_sec_32;   // large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not
			       // fit in the Number of Sectors entry at 0x13
} __attribute__((packed)) bpb_t;

typedef struct ebpb {
	uint8_t drv_num;
	uint8_t reserved1;
	uint8_t boot_sig;
	uint32_t vol_id;
	uint8_t vol_lab[11];
	uint8_t fil_sys_type[8];
} __attribute__((packed)) ebpb_t;

typedef struct fat16_dir_entry {
	uint8_t file_name[11];	    // 8.3 file name
	uint8_t attributes;	    // attributes of the file
	uint8_t reserved;	    // reserved for use by Windows NT
	uint8_t creation_time_ms;   // creation time in hundredths of a second
	uint16_t create_time;	    // time that the file was created (in the format described)
	uint16_t create_date;	    // date on which the file was created (in the format described)
	uint16_t last_access_date;  // last accessed date (in the format described)
	uint16_t high_cluster;	    // high 16 bits of the first cluster number
	uint16_t modification_time; // last modification time (in the format described)
	uint16_t modification_date; // last modification date (in the format described)
	uint16_t low_cluster;	    // low 16 bits of the first cluster number
	uint32_t file_size;	    // size of the file in bytes
} __attribute__((packed)) fat16_dir_entry_t;

typedef struct fat16_internal_header {
	bpb_t bpb;
	ebpb_t ebpb;
} __attribute__((packed)) fat16_internal_header_t;

typedef struct fat16_time {
	int32_t hour;
	int32_t minute;
	int32_t second;
} fat16_time_t;

typedef struct fat16_date {
	int32_t day;
	int32_t month;
	int32_t year;
} fat16_date_t;

typedef enum fat16_entry {
	FAT16_ENTRY_TYPE_FILE,
	FAT16_ENTRY_TYPE_DIRECTORY,
} fat16_entry_t;

typedef struct fat16_folder {
	struct fat16_dir_entry* entry;
	uint32_t total;
	uint32_t start_sector;
	uint32_t end_sector;
} fat16_folder_t;

typedef struct fat16_node {
	union {
		fat16_dir_entry_t* file; // Pointer to the file
		fat16_folder_t* dir;	 // Pointer to the folder
	};
	fat16_entry_t type;
} fat16_node_t;

typedef struct fat16_fd {
	fat16_node_t* entry;
	uint32_t pos;
} fat16_fd_t;

fat16_internal_header_t fat16_header = {
    .bpb =
	{
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
    .ebpb =
	{
	    .drv_num = 0,
	    .reserved1 = 0,
	    .boot_sig = 0,
	    .vol_id = 0,
	    .vol_lab = {0},
	    .fil_sys_type = {0},
	},
};

fs_t fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .read_cb = 0x0,
    .close_cb = 0x0,
    .stat_cb = 0x0,
    .seek_cb = 0x0,
    .name = "FAT16",
};

/* PUBLIC API */
fs_t* fat16_init(void);
int32_t fat16_resolve(ata_t* dev);
void* fat16_open(ata_t* dev, pathnode_t* path, uint8_t mode);
size_t fat16_read(ata_t* dev, void* descriptor, uint8_t* buffer, size_t n_bytes, size_t n_blocks);
int32_t fat16_close(void* internal);
int32_t fat16_stat(ata_t* dev, void* internal, vstat_t* vstat);
int32_t fat16_seek(void* internal, const uint32_t offset, const uint8_t origin);
size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks);
void fat16_dump_root_dir_entry_at(ata_t* dev, const int32_t i);

/* INTERNAL API */
static uint32_t _convert_data_cluster_to_sector(const uint32_t cluster);
static uint16_t _read_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster);
static fat16_time_t _convert_time(const uint16_t time);
static fat16_date_t _convert_date(const uint16_t date);
static uint16_t _fat_date_to_uint16(const fat16_date_t dateInfo);
static bool _validate_fat16_header(const fat16_internal_header_t* header);
static void _dump_fat16_ebpb_header(const ebpb_t* ebpb, const char* msg, const int32_t delay);
static void _dump_fat16_base_header(const bpb_t* bpb, const char* msg, const int32_t delay);
static uint32_t _calc_root_dir_area_offset(const bpb_t* bpb);
static uint32_t _calc_root_dir_area_absolute(const bpb_t* bpb, const uint32_t partition_offset);
static uint32_t _calc_fat_area_offset(const bpb_t* bpb);
static uint32_t _calc_fat_area_absolute(const bpb_t* bpb, const uint32_t partition_offset);
static void _print_dir_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
static void _print_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
static void _dump_root_dir_entries(const bpb_t* bpb, stream_t* stream_t);
static void _convert_userland_filename_to_native(uint8_t* native, const uint8_t* userland);
static uint32_t _combine_clusters(const uint16_t high_cluster, const uint16_t low_cluster);
static fat16_dir_entry_t* _get_entry_in_subdir(ata_t* dev, const uint16_t start_cluster, const uint8_t* native_file_name, fat16_folder_t* fat16_folder,
					       fat16_dir_entry_t* fat16_dir_entry);
static fat16_dir_entry_t* _get_root_dir_entry(ata_t* dev, fat16_dir_entry_t* root_dir_entry, pathnode_t* path_identifier);
static fat16_node_t* _get_entry(ata_t* dev, pathnode_t* path);
void* fat16_open(ata_t* dev, pathnode_t* path, uint8_t mode);
static uint32_t _get_start_cluster_from_descriptor(fat16_fd_t* fat16_descriptor);
static void _set_stat(fat16_fd_t* fat16_descriptor, ata_t* dev, vstat_t* vstat, uint32_t max_cluster_size_bytes, uint32_t used_blocks);
static uint16_t _count_allocated_fat_blocks_in_chain(stream_t* fat_stream, const uint16_t max_cluster_size_bytes, const uint32_t start_cluster,
						     const uint32_t partition_offset);
static fat16_node_t* _create_fat16_file(ata_t* dev, pathnode_t* path);

static uint32_t _convert_data_cluster_to_sector(const uint32_t cluster)
{
	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
	const uint32_t data_start_sector = fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16) + root_dir_sectors;
	return data_start_sector + ((cluster - 2) * fat16_header.bpb.sec_per_clus);
};

static uint16_t _read_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster)
{
	uint8_t fat_entry[2] = {};
	const uint32_t fat_entry_offset = partition_offset + fat16_header.bpb.rsvd_sec * fat16_header.bpb.byts_per_sec + curr_cluster * 2;
	stream_seek(fat_stream, fat_entry_offset);
	stream_read(fat_stream, fat_entry, 2 * sizeof(uint8_t));
	return ((uint16_t)fat_entry[0]) | ((uint16_t)fat_entry[1] << 8);
};

static fat16_time_t _convert_time(const uint16_t time)
{
	fat16_time_t time_info = {};
	time_info.second = (time & 0x1F) * 2;
	time_info.minute = (time >> 5) & 0x3F;
	time_info.hour = (time >> 11) & 0x1F;
	return time_info;
};

static fat16_date_t _convert_date(const uint16_t date)
{
	fat16_date_t date_info = {};
	date_info.day = date & 0x1F;
	date_info.month = (date >> 5) & 0x0F;
	const uint16_t original_year = (date >> 9) & 0x7F;
	date_info.year = (original_year == 0) ? 0 : (original_year + 1980);
	return date_info;
};

static uint16_t _fat_date_to_uint16(const fat16_date_t dateInfo)
{
	uint16_t fat16_date = 0;
	fat16_date |= ((dateInfo.year - 1980) & 0x7F) << 9;
	fat16_date |= (dateInfo.month & 0x0F) << 5;
	fat16_date |= (dateInfo.day & 0x1F);
	return fat16_date;
};

fs_t* fat16_init(void)
{
	fat16.resolve_cb = fat16_resolve;
	fat16.open_cb = fat16_open;
	fat16.read_cb = fat16_read;
	fat16.close_cb = fat16_close;
	fat16.stat_cb = fat16_stat;
	fat16.seek_cb = fat16_seek;
	fat16.write_cb = fat16_write;
	return &fat16;
};

static bool _validate_fat16_header(const fat16_internal_header_t* header)
{
	const bool has_signature = (header->bpb.jmp[0] == 0xEB && header->bpb.jmp[2] == 0x90);
	const bool has_header =
	    has_signature && (header->bpb.byts_per_sec == 512) && (header->bpb.sec_per_clus >= 1) && (header->bpb.num_fats == 2) && (header->bpb.media != 0);

	if (!has_header) {
		return false;
	};
	const bool has_ebpb = (header->ebpb.boot_sig == 0x29) && (header->ebpb.drv_num == 0x80);

	if (has_ebpb) {
		const uint32_t total_sectors = (header->bpb.tot_sec_32 != 0) ? header->bpb.tot_sec_32 : header->bpb.tot_sec_16;
		const uint32_t disk_size = total_sectors * header->bpb.byts_per_sec;

		if (total_sectors > disk_size) {
			return false;
		};
	};
	return true;
};

static void _dump_fat16_ebpb_header(const ebpb_t* ebpb, const char* msg, const int32_t delay)
{
	uint8_t vol_lab[12] = {};
	uint8_t fil_sys_type[9] = {};
	memcpy(vol_lab, ebpb->vol_lab, 11);
	vol_lab[11] = '\0';
	memcpy(fil_sys_type, ebpb->fil_sys_type, 7);
	fil_sys_type[7] = '\0';

	printf("\n=============================================\n");
	printf("%s\n", msg);
	printf("---------------------------------------------\n");
	printf(" Drive Number    : 0x%x\n", ebpb->drv_num);
	printf(" Reserved1       : 0x%x\n", ebpb->reserved1);
	printf(" Boot Signature  : %d\n", ebpb->boot_sig);
	printf(" Volume ID       : 0x%x\n", ebpb->vol_id);
	printf(" Volume Label    : %s\n", vol_lab);
	printf(" File System Type: %s\n", fil_sys_type);
	printf("=============================================\n");
	return;
};

static void _dump_fat16_base_header(const bpb_t* bpb, const char* msg, const int32_t delay)
{
	printf("\n=============================================\n");
	printf("%s\n", msg);
	printf("---------------------------------------------\n");
	printf(" Jump Code       : 0x%x 0x%x 0x%x\n", bpb->jmp[0], bpb->jmp[1], bpb->jmp[2]);
	printf(" OEM Name        : %s\n", bpb->oem);
	printf(" Bytes Per Sec   : %d\n", bpb->byts_per_sec);
	printf(" Sec Per Cluster : %d\n", bpb->sec_per_clus);
	printf(" Reserved Sectors: %d\n", bpb->rsvd_sec);
	printf(" Number of FATs  : %d\n", bpb->num_fats);
	printf(" Root Entry Count: %d\n", bpb->root_ent_cnt);
	printf(" Total Sectors16 : %d\n", bpb->tot_sec_16);
	printf(" Media Type      : 0x%x\n", bpb->media);
	printf(" FAT Size 16     : %d\n", bpb->fatsz16);
	printf(" Sectors Per Trk : %d\n", bpb->sec_per_trk);
	printf(" Number of Heads : %d\n", bpb->num_heads);
	printf(" Hidden Sectors  : %d\n", bpb->hidd_sec);
	printf(" Total Sectors32 : %d\n", bpb->tot_sec_32);
	printf("=============================================\n");
	return;
};

static uint32_t _calc_root_dir_area_offset(const bpb_t* bpb)
{
	const uint32_t root_directory_offset = bpb->byts_per_sec * (bpb->rsvd_sec + (bpb->num_fats * bpb->fatsz16));
	return root_directory_offset;
};

static uint32_t _calc_root_dir_area_absolute(const bpb_t* bpb, const uint32_t partition_offset)

{
	const uint32_t root_dir_area_absolute = partition_offset + _calc_root_dir_area_offset(bpb);
	return root_dir_area_absolute;
};

static uint32_t _calc_fat_area_offset(const bpb_t* bpb)
{
	const uint32_t fat_area_offset = bpb->rsvd_sec * bpb->byts_per_sec;
	return fat_area_offset;
};

static uint32_t _calc_fat_area_absolute(const bpb_t* bpb, const uint32_t partition_offset)
{
	const uint32_t fat_area_absolute = partition_offset + _calc_fat_area_offset(bpb);
	return fat_area_absolute;
};

static void _print_dir_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay)
{
	const fat16_time_t create_time = _convert_time(entry->create_time);
	const fat16_date_t create_date = _convert_date(entry->create_date);
	const fat16_date_t last_access_date = _convert_date(entry->last_access_date);
	const fat16_time_t mod_time = _convert_time(entry->modification_time);
	const fat16_date_t mod_date = _convert_date(entry->modification_date);

	uint8_t buffer[11] = {};
	memcpy(buffer, entry->file_name, 11);

	printf("==========================\n");
	printf("=   RootDirEntry %d:\n", i);
	printf("==========================\n");
	printf("=   Filename: %s\n", buffer);
	printf("=   Attributes: 0x%x\n", entry->attributes);
	printf("=   Creation Time: %d:%d:%d\n", create_time.hour, create_time.minute, create_time.second);
	printf("=   Creation Date: %d.%d.%d\n", create_date.day, create_date.month, create_date.year);
	printf("=   Last Access Date: %d.%d.%d\n", last_access_date.day, last_access_date.month, last_access_date.year);
	printf("=   High Cluster: %d\n", entry->high_cluster);
	printf("=   Modification Time: %d:%d:%d\n", mod_time.hour, mod_time.minute, mod_time.second);
	printf("=   Modification Date: %d.%d.%d\n", mod_date.day, mod_date.month, mod_date.year);
	printf("=   Low Cluster: %d\n", entry->low_cluster);
	printf("=   File Size: %d Bytes\n", entry->file_size);
	printf("==========================\n");
	busy_wait(delay);
	return;
};

static void _print_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay)
{
	if (entry->file_name[0] == 0x00) {
		return;
	};
	printf("==========================\n");
	printf("=   RootDirLFNEntry %d:\n", i);
	printf("==========================\n");
	printf("=   LFN: %s (Long File Name NOT SUPPORTED)\n", "-");
	printf("==========================\n");
	// busy_wait(delay);
	return;
};

static void _dump_root_dir_entries(const bpb_t* bpb, stream_t* stream_t)
{
	const uint32_t root_dir_size = bpb->root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t root_dir_entries = root_dir_size / sizeof(fat16_dir_entry_t);

	for (size_t i = 0; i < bpb->root_ent_cnt; i++) {
		fat16_dir_entry_t entry = {};
		const int32_t res = stream_read(stream_t, (uint8_t*)&entry, sizeof(fat16_dir_entry_t));

		if (res != 0) {
			printf("FAT16 Error: Failed to read FAT16 Header\n");
			return;
		};

		if (entry.attributes == LFN) {
			_print_lfn_entry(i, &entry, FAT16_DEBUG_DELAY);
			continue;
		};
		if (entry.file_name[0] != 0) {
			_print_dir_entry(i, &entry, FAT16_DEBUG_DELAY);
		};
	};
	return;
};

int32_t fat16_resolve(ata_t* dev)
{
	stream_t header_stream = {};
	const uint32_t partition_offset = 0x100000;
	stream_init(&header_stream, dev);
	stream_seek(&header_stream, partition_offset);
	const int32_t res = stream_read(&header_stream, (uint8_t*)&fat16_header, sizeof(fat16_internal_header_t));

	if (res != 0) {
		printf("FAT16 Error: Failed to read FAT16 Header\n");
		return -EIO;
	};

	if (!_validate_fat16_header(&fat16_header)) {
		printf("FAT16 Error: Invalid FAT16 Header\n");
		return -EIO;
	};
	_dump_fat16_base_header(&fat16_header.bpb, "", 0);
	_dump_fat16_ebpb_header(&fat16_header.ebpb, "", 0);
	const uint32_t root_dir_area_offset = _calc_root_dir_area_offset(&fat16_header.bpb);
	const uint32_t root_dir_area_absolute = _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t root_dir_area_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t root_dir_area_entries = root_dir_area_size / sizeof(fat16_dir_entry_t);

	printf("Root Dir Area Offset: 0x%x\n", root_dir_area_offset);
	printf("Root Dir Area Absolute: 0x%x\n", root_dir_area_absolute);
	printf("Root Dir Area Size: %d\n", root_dir_area_size);
	printf("Root Dir Area Entries: %d\n", root_dir_area_entries);

	stream_t root_dir = {};
	stream_init(&root_dir, dev);
	stream_seek(&root_dir, root_dir_area_absolute);
	// _dump_root_dir_entries(&fat16_header.bpb, &root_dir);

	const uint32_t fat_area_offset = _calc_fat_area_offset(&fat16_header.bpb);
	const uint32_t fat_area_absolute = _calc_fat_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_area_size = fat16_header.bpb.fatsz16 * fat16_header.bpb.byts_per_sec;
	const uint32_t fat_area_entries = fat_area_size / sizeof(uint16_t);

	printf("FAT Area Offset: 0x%x\n", fat_area_offset);
	printf("FAT Area Absolute: 0x%x\n", fat_area_absolute);
	printf("FAT Area Size: %d\n", fat_area_size);
	printf("FAT Area Entries: %d\n", fat_area_entries);
	// busy_wait(FAT16_DEBUG_DELAY);

	const uint32_t total_sectors = fat16_header.bpb.tot_sec_16 == 0 ? fat16_header.bpb.tot_sec_32 : fat16_header.bpb.tot_sec_16;
	printf("Total Sectors: %d\n", total_sectors);
	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
	printf("Root Dir Sectors: %d\n", root_dir_sectors);
	const uint32_t first_data_sector = fat16_header.bpb.rsvd_sec + fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + (root_dir_sectors);
	printf("First Data Sector: %d\n", first_data_sector);
	const uint32_t first_fat_sector = fat16_header.bpb.rsvd_sec;
	printf("First FAT Sector: %d\n", first_fat_sector);
	const uint32_t data_sectors = total_sectors - (fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + root_dir_sectors));
	printf("Data Sectors: %d\n", data_sectors);
	const uint32_t total_clusters = data_sectors / fat16_header.bpb.sec_per_clus;
	printf("Total Clusters: %d\n", total_clusters);

	if (total_clusters > FAT16_MAX_CLUSTERS || total_clusters < FAT16_MIN_CLUSTERS) {
		printf("FAT16 Error: Invalid FAT16 Header\n");
		return -EIO;
	};
	return 0;
};

static void _convert_fat16_native_to_userland(char* userland, const uint8_t* native)
{
	if (!userland || !native) {
		return;
	};
	memset(userland, 0, 13);

	size_t i = 0;

	while (i < 8 && native[i] != ' ') {
		userland[i] = native[i];
		i++;
	};

	if (native[8] != ' ') {
		userland[i++] = '.';
		size_t j = 0;

		while (j < 3 && native[8 + j] != ' ') {
			userland[i++] = native[8 + j];
			j++;
		};
	};
	userland[i] = '\0';
	return;
};

static void _convert_userland_filename_to_native(uint8_t* native, const uint8_t* userland)
{
	uint8_t i, j = 0;

	for (i = 0; i < 11; i++) {
		native[i] = ' ';
	};

	for (i = 0; i < 8 && userland[i] != '\0' && userland[i] != '.'; i++) {
		native[i] = userland[i];
	};

	if (userland[i] == '.') {
		for (j = 0, i++; j < 3 && userland[i] != '\0'; i++, j++) {
			native[8 + j] = userland[i];
		};
	};
	return;
};

static uint32_t _combine_clusters(const uint16_t high_cluster, const uint16_t low_cluster)
{
	const uint32_t cluster = (high_cluster << 16) | low_cluster;
	return cluster;
};

static fat16_dir_entry_t* _get_entry_in_subdir(ata_t* dev, const uint16_t start_cluster, const uint8_t* native_file_name, fat16_folder_t* fat16_folder,
					       fat16_dir_entry_t* fat16_dir_entry)
{
	const uint32_t partition_offset = 0x100000;

	stream_t fat_stream = {};
	stream_t data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);

	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

	uint8_t buffer[max_cluster_size_bytes];
	uint16_t curr_cluster = start_cluster;

	const uint32_t num_entries = max_cluster_size_bytes / sizeof(fat16_dir_entry_t);
	uint32_t total_entries = 0;

	while (curr_cluster <= FAT16_VALUE_END_OF_CHAIN) {
		const uint32_t sector = _convert_data_cluster_to_sector(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		const int32_t res = stream_read(&data_stream, buffer, max_cluster_size_bytes);

		if (res < 0) {
			printf("[Stream] ERROR: Failed to read Cluster %d (ATA Error %d)\n", curr_cluster, res);
			return 0x0;
		};
		uint8_t* curr_dir_entry = buffer;

		for (int i = 0; i < num_entries; i++) {
			char tmp[11] = {};
			memcpy(tmp, ((fat16_dir_entry_t*)curr_dir_entry)->file_name, 11);

			if (memcmp(tmp, native_file_name, 11) == 0) {
				const uint32_t start_sector = _convert_data_cluster_to_sector(start_cluster);
				const uint32_t end_sector = _convert_data_cluster_to_sector(curr_cluster);
				fat16_folder->start_sector = start_sector;
				fat16_folder->end_sector = end_sector;
				fat16_folder->total = total_entries;
				fat16_dir_entry_t* entry = (fat16_dir_entry_t*)curr_dir_entry;
				memcpy(fat16_dir_entry, curr_dir_entry, sizeof(fat16_dir_entry_t));
				return fat16_dir_entry;
			};
			curr_dir_entry += sizeof(fat16_dir_entry_t);
			total_entries++;
		};
		curr_cluster = _read_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	fat16_folder->total = total_entries;
	return 0x0;
};

// Searches for a specific file or directory in the FAT16 root directory to locate its entry for further access or manipulation
static fat16_dir_entry_t* _get_root_dir_entry(ata_t* dev, fat16_dir_entry_t* root_dir_entry, pathnode_t* path_identifier)
{
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_area_absolute = _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);

	stream_t stream_t = {};
	stream_init(&stream_t, dev);
	stream_seek(&stream_t, root_dir_area_absolute);

	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
	const uint32_t first_data_sector = fat16_header.bpb.rsvd_sec + fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + (root_dir_sectors);
	const uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
	const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	uint8_t buffer[root_dir_size];

	const int32_t res = stream_read(&stream_t, buffer, root_dir_size);

	if (res < 0) {
		printf("[Stream] ERROR: Failed to read Root Dir (ATA Error %d)\n", res);
		return 0x0;
	};
	fat16_dir_entry_t* curr_root_entry = (fat16_dir_entry_t*)buffer;

	for (size_t i = 0; i < fat16_header.bpb.root_ent_cnt; i++, curr_root_entry++) {
		if (curr_root_entry->file_name[0] != 0x0) {
			uint8_t native_dir_name[11] = {};
			_convert_userland_filename_to_native(native_dir_name, (uint8_t*)path_identifier);

			if (memcmp(curr_root_entry->file_name, native_dir_name, 11) == 0) {
				memcpy(root_dir_entry, curr_root_entry, sizeof(fat16_dir_entry_t));
				break;
			};
		};
	};
	return root_dir_entry;
};

static fat16_node_t* _get_entry(ata_t* dev, pathnode_t* path)
{
	pathnode_t* curr = path;

	while (curr && curr->next) {
		curr = curr->next;
	};
	uint8_t native[11] = {};
	_convert_userland_filename_to_native(native, (uint8_t*)curr);

	fat16_dir_entry_t root_dir_entry = {};
	_get_root_dir_entry(dev, &root_dir_entry, path);

	const uint32_t root_dir_entry_cluster = _combine_clusters(root_dir_entry.high_cluster, root_dir_entry.low_cluster);
	uint32_t start_cluster = root_dir_entry_cluster;

	fat16_folder_t folder = {};
	fat16_dir_entry_t entry = {};
	_get_entry_in_subdir(dev, start_cluster, native, &folder, &entry);

	if (entry.file_name[0] && (entry.attributes & DIRECTORY)) {
		fat16_node_t* node = kzalloc(sizeof(fat16_node_t));
		node->dir = kzalloc(sizeof(fat16_folder_t));
		node->dir->entry = kzalloc(sizeof(fat16_dir_entry_t));
		node->type = FAT16_ENTRY_TYPE_DIRECTORY;
		memcpy(node->dir->entry, &entry, sizeof(fat16_dir_entry_t));
		memcpy(node->dir, &folder, sizeof(fat16_folder_t));
		return node;
	};

	if (entry.file_name[0] && (entry.attributes & ARCHIVE)) {
		fat16_node_t* node = kzalloc(sizeof(fat16_node_t));
		node->file = kzalloc(sizeof(fat16_dir_entry_t));
		node->type = FAT16_ENTRY_TYPE_FILE;
		memcpy(node->file, &entry, sizeof(fat16_dir_entry_t));
		return node;
	};
	return 0x0;
};

void* fat16_open(ata_t* dev, pathnode_t* path, uint8_t mode)
{
	if (mode != READ && mode != WRITE) {
		printf("[FAT16] Unsupported Mode\n");
		return 0x0;
	};

	if (!dev || !dev->fs || !path) {
		printf("[FAT16] Device, Filesystem or Path not initialized\n");
		return 0x0;
	};
	fat16_node_t* entry = _get_entry(dev, path);

	if (!entry && mode == WRITE) {
		printf("[FAT16] File not found - Creating FAT16 Entry\n");
		entry = _create_fat16_file(dev, path);

		if (!entry) {
			printf("[FAT16] ERROR: File Creation failed\n");
			return 0x0;
		};
	};
	fat16_fd_t* fd = kzalloc(sizeof(fat16_fd_t));

	if (!fd) {
		printf("[FAT16] Memory Allocation failed\n");
		kfree(fd);
		return 0x0;
	};
	fd->entry = entry;
	fd->pos = 0;
	return fd;
};

static uint32_t _get_start_cluster_from_descriptor(fat16_fd_t* fat16_descriptor)
{
	uint32_t start_cluster = 0;

	switch (fat16_descriptor->entry->type) {
	case FAT16_ENTRY_TYPE_DIRECTORY: {
		start_cluster = _combine_clusters(fat16_descriptor->entry->dir->entry->high_cluster, fat16_descriptor->entry->dir->entry->low_cluster);
		break;
	};
	case FAT16_ENTRY_TYPE_FILE: {
		start_cluster = _combine_clusters(fat16_descriptor->entry->file->high_cluster, fat16_descriptor->entry->file->low_cluster);
		break;
	};
	default: {
		break;
	};
	};
	return start_cluster;
};

// Read data from a FAT16 filesystem
size_t fat16_read(ata_t* dev, void* descriptor, uint8_t* buffer, const size_t n_bytes, const size_t n_blocks)
{
	// Define the offset of the partition
	const uint32_t partition_offset = 0x100000;
	// Essentially to seek through the streams
	stream_t fat_stream = {}, data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);
	// Cast the vfs internal back to the concrete fat16 descriptor
	fat16_fd_t* fat16_descriptor = descriptor;
	// Maximum cluster size in bytes of a single FAT16 cluster
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
	// The given file descriptor holds a pointer to the root dir entry, which holds the fat16 start cluster number to read
	const uint32_t start_cluster = _get_start_cluster_from_descriptor(fat16_descriptor);
	// Determine the starting cluster for reading
	uint16_t curr_cluster = (fat16_descriptor->pos / max_cluster_size_bytes) + start_cluster;
	const uint16_t first_cluster_offset = fat16_descriptor->pos % max_cluster_size_bytes;
	// Initialize bytes_read for tracking read progress and return to the  caller of the function the amount of readed bytes
	size_t bytes_read = 0;
	// Initialize remaining_bytes for tracking the termination of the following read loop
	size_t remaining_bytes = n_bytes * n_blocks - bytes_read;
	// Limit read_size to the smaller of remaining_bytes and the max cluster size in FAT16 filesystem
	size_t read_size = remaining_bytes < max_cluster_size_bytes ? remaining_bytes : max_cluster_size_bytes;

	while (remaining_bytes) {
		// Convert the logical fat16 cluster number to a physical sector
		const uint32_t sector = _convert_data_cluster_to_sector(curr_cluster);
		// Use the previously calculated sector and add the partition offset and the first cluster to get the data position on die ata device
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec) + first_cluster_offset;
		// Seek to the data position and read data into the buffer
		stream_seek(&data_stream, data_pos);
		// Ensure that new data is appended to the buffer + bytes_read ensures the correct position so that previously written data is not overwritten
		const int32_t res = stream_read(&data_stream, buffer + bytes_read, read_size);

		if (res < 0) {
			printf("StreamError: An error occurred while reading FAT16\n");
			return bytes_read;
		};
		// Update read progress
		bytes_read += read_size;
		fat16_descriptor->pos += read_size;
		remaining_bytes -= read_size;
		// Determine the next cluster from the FAT table
		const uint16_t next_cluster = _read_next_cluster(&fat_stream, partition_offset, curr_cluster);
		// Check if end of file is reached, if yes, exit loop
		if (next_cluster >= FAT16_VALUE_END_OF_CHAIN) {
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

int32_t fat16_close(void* internal)
{
	int32_t res = 0;
	fat16_fd_t* fat16_descriptor = (fat16_fd_t*)internal;

	if (fat16_descriptor == 0x0) {
		res = -EINVAL;
		return res;
	};

	if (!fat16_descriptor->entry) {
		kfree(fat16_descriptor);
		return 0;
	};
	if (fat16_descriptor->entry->type == FAT16_ENTRY_TYPE_DIRECTORY) {
		if (fat16_descriptor->entry->dir) {
			if (fat16_descriptor->entry->dir->entry) {
				kfree(fat16_descriptor->entry->dir->entry);
			};
			kfree(fat16_descriptor->entry->dir);
		};
	} else if (fat16_descriptor->entry->type == FAT16_ENTRY_TYPE_FILE) {
		if (fat16_descriptor->entry->file) {
			kfree(fat16_descriptor->entry->file);
		};
	};
	kfree(fat16_descriptor->entry);
	kfree(fat16_descriptor);
	return 0;
};

static void _set_stat(fat16_fd_t* fat16_descriptor, ata_t* dev, vstat_t* vstat, uint32_t max_cluster_size_bytes, uint32_t used_blocks)
{
	switch (fat16_descriptor->entry->type) {
	case FAT16_ENTRY_TYPE_DIRECTORY: {
		memcpy(vstat->st_dev, dev->dev, sizeof(char) * 2);
		vstat->st_mode = READ;
		vstat->st_size = fat16_descriptor->entry->dir->entry->file_size;
		vstat->st_blksize = max_cluster_size_bytes;
		vstat->st_blocks = (used_blocks * max_cluster_size_bytes) / dev->sector_size;
		vstat->st_atime = fat16_descriptor->entry->dir->entry->last_access_date;
		vstat->st_mtime = fat16_descriptor->entry->dir->entry->modification_date;
		vstat->st_ctime = fat16_descriptor->entry->dir->entry->creation_time_ms;
		break;
	};
	case FAT16_ENTRY_TYPE_FILE: {
		memcpy(vstat->st_dev, dev->dev, sizeof(char) * 2);
		vstat->st_mode = READ;
		vstat->st_size = fat16_descriptor->entry->file->file_size;
		vstat->st_blksize = max_cluster_size_bytes;
		vstat->st_blocks = (used_blocks * max_cluster_size_bytes) / dev->sector_size;
		vstat->st_atime = fat16_descriptor->entry->file->last_access_date;
		vstat->st_mtime = fat16_descriptor->entry->file->modification_date;
		vstat->st_ctime = fat16_descriptor->entry->file->creation_time_ms;
		break;
	};
	default:
		break;
	};
	return;
};

static uint16_t _count_allocated_fat_blocks_in_chain(stream_t* fat_stream, const uint16_t max_cluster_size_bytes, const uint32_t start_cluster,
						     const uint32_t partition_offset)
{
	uint16_t used_blocks = 1;

	uint16_t curr_cluster = (0 / max_cluster_size_bytes) + start_cluster;
	uint16_t next_cluster = _read_next_cluster(fat_stream, partition_offset, curr_cluster);

	while (next_cluster < FAT16_VALUE_END_OF_CHAIN) {
		used_blocks++;
		curr_cluster = next_cluster;
		next_cluster = _read_next_cluster(fat_stream, partition_offset, curr_cluster);
	};
	return used_blocks;
};

int32_t fat16_stat(ata_t* dev, void* internal, vstat_t* vstat)
{
	int32_t res = 0;

	if (dev == 0x0 || internal == 0x0 || vstat == 0x0) {
		res = -EINVAL;
		return res;
	};
	fat16_fd_t* fat16_descriptor = (fat16_fd_t*)internal;
	// Should be there, because we must follow the fat cluster chain, to find the sum of all the allocated blocks
	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	// Define the offset of the partition
	const uint32_t partition_offset = 0x100000;
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
	const uint32_t start_cluster = _get_start_cluster_from_descriptor(fat16_descriptor);
	// Follow the cluster chain from the start 0 from the starting cluster and keep track of allocated blocks
	const uint16_t used_blocks = _count_allocated_fat_blocks_in_chain(&fat_stream, max_cluster_size_bytes, start_cluster, partition_offset);
	_set_stat(fat16_descriptor, dev, vstat, max_cluster_size_bytes, used_blocks);
	return res;
};

int32_t fat16_seek(void* internal, const uint32_t offset, const uint8_t origin)
{
	int32_t res = 0;
	fat16_fd_t* fat16_descriptor = (fat16_fd_t*)internal;

	if (offset >= fat16_descriptor->entry->dir->entry->file_size) {
		res = -EIO;
		return res;
	};

	switch (origin) {
	case SEEK_SET: {
		fat16_descriptor->pos = offset;
		break;
	};
	case SEEK_END: {
		res = ENOENT;
		break;
	};
	case SEEK_CUR: {
		fat16_descriptor->pos += offset;
		break;
	};
	default: {
		res = -EINVAL;
		break;
	};
	};
	return res;
};

static uint32_t _get_dir_cluster(ata_t* dev, pathnode_t* path)
{
	if (!dev || !path) {
		return 0;
	};
	const uint32_t partition_offset = 0x100000;
	uint32_t curr_cluster = _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
	pathnode_t* curr = path;

	while (curr) {
		fat16_folder_t parent_dir = {};
		fat16_dir_entry_t dir_entry = {};

		if (!_get_entry_in_subdir(dev, curr_cluster, (uint8_t*)curr->identifier, &parent_dir, &dir_entry)) {
			printf("[FAT16] ERROR: Directory '%s' not found!\n", curr->identifier);
			return 0;
		};
		curr_cluster = _combine_clusters(dir_entry.high_cluster, dir_entry.low_cluster);

		if (curr_cluster == 0) {
			printf("[FAT16] ERROR: Cluster Calculation failed for '%s'\n", curr->identifier);
			return 0;
		};
		curr = curr->next;
	};
	return curr_cluster;
};

static uint32_t _get_free_dir_entry(ata_t* dev, uint32_t parent_cluster) { return 0; };

static uint32_t _find_free_root_dir_entry(ata_t* dev)
{
	if (!dev) {
		return 0x0;
	};
	// Calculate the position of the root directory area
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_sector = _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t num_entries = root_dir_size / sizeof(fat16_dir_entry_t);
	// Load the root directory into memory
	stream_t stream;
	stream_init(&stream, dev);
	stream_seek(&stream, root_dir_sector);
	uint8_t buffer[root_dir_size];
	const int32_t res = stream_read(&stream, buffer, root_dir_size);

	if (res < 0) {
		printf("[FAT16] ERROR: Failed to read Root Directory.\n");
		return 0x0;
	};
	// Search for a free entry (`0x00` or `0xE5`)
	fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;

	for (uint32_t i = 0; i < num_entries; i++, entry++) {
		if (entry->file_name[0] == 0x00 || entry->file_name[0] == 0xE5) {
			printf("[FAT16] INFO: Found free Root Directory Entry at Index %d\n", i);
			return root_dir_sector + (i * sizeof(fat16_dir_entry_t));
		};
	};
	// If no more space → Root directory is full!
	printf("[FAT16] ERROR: Root Directory is FULL. Cannot create new File.\n");
	return 0x0;
};

static uint16_t _find_free_cluster(stream_t* fat_stream, const uint32_t partition_offset)
{
	uint8_t fat_entry[2] = {};

	for (uint16_t cluster = 2; cluster < FAT16_MAX_CLUSTERS; cluster++) {
		uint32_t fat_entry_offset = partition_offset + fat16_header.bpb.rsvd_sec * fat16_header.bpb.byts_per_sec + cluster * 2;
		stream_seek(fat_stream, fat_entry_offset);
		const int32_t res = stream_read(fat_stream, fat_entry, 2);

		if (res < 0) {
			printf("[FAT16] ERROR: Failed to read.\n");
			return 0x0;
		};
		uint16_t value = ((uint16_t)fat_entry[0]) | ((uint16_t)fat_entry[1] << 8);

		if (value == 0x0000) {
			return cluster;
		};
	};
	return 0;
};

static bool _write_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry) { return false; };

static bool _write_root_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry)
{
	if (!dev || !new_entry) {
		printf("[FAT16] ERROR: Invalid parameters in _write_root_dir_entry\n");
		return false;
	};
	uint32_t entry_offset = entry_sector;
	// Open a write stream to the root directory position
	stream_t stream = {};
	stream_init(&stream, dev);
	stream_seek(&stream, entry_offset);
	// Write the new file entry to the calculated position
	const int32_t res = stream_write(&stream, (uint8_t*)new_entry, sizeof(fat16_dir_entry_t));

	if (res < 0) {
		printf("[FAT16] ERROR: Failed to write Root Directory Entry\n");
		return false;
	};
	printf("[FAT16] SUCCESS: File Entry written to Root Directory at Sector %d\n", entry_sector);
	return true;
};

#include <stdio.h>

void _dump_fat_table(ata_t* dev)
{
	if (!dev) {
		printf("[FAT16] ERROR: Invalid Dev\n");
		return;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t fat_offset = _calc_fat_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_size = fat16_header.bpb.fatsz16 * fat16_header.bpb.byts_per_sec;
	const uint32_t fat_entries = fat_size / 2;

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	stream_seek(&fat_stream, fat_offset);

	uint16_t fat_entry = 0;
	printf("[FAT16] Dumping FAT Table:\n");
	for (uint32_t cluster = 2; cluster < FAT16_MAX_CLUSTERS; cluster++) {
		const int32_t res = stream_read(&fat_stream, (uint8_t*)&fat_entry, sizeof(fat_entry));

		if (res < 0) {
			printf("[Stream] Failed to read from %d\n", cluster);
			return;
		};
		printf("Cluster %d: 0x%x\n", cluster, fat_entry);

		if (fat_entry == FAT16_VALUE_END_OF_CHAIN) {
			printf("Cluster %u: End of chain (EOF)\n", cluster);
		} else if (fat_entry == FAT16_VALUE_BAD_CLUSTER) {
			printf("Cluster %u: Bad cluster\n", cluster);
		} else if (fat_entry == FAT16_VALUE_FREE) {
			printf("Cluster %u: Free cluster\n", cluster);
		};
	};
	return;
};

static bool _update_fat_entry(ata_t* dev, uint32_t cluster, uint16_t value)
{
	if (!dev || cluster < 2 || cluster >= FAT16_MAX_CLUSTERS) {
		return false;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t fat_offset = _calc_fat_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_entry_offset = fat_offset + cluster * 2;

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	stream_seek(&fat_stream, fat_entry_offset);

	uint8_t fat_entry[2] = {(uint8_t)(value & 0xFF), (uint8_t)((value >> 8) & 0xFF)};
	int32_t res = stream_write(&fat_stream, fat_entry, sizeof(uint16_t));

	if (res < 0) {
		printf("[FAT16] ERROR: Failed to update FAT Entry for Cluster %d\n", cluster);
		return false;
	};
	printf("[FAT16] SUCCESS: Updated FAT entry for Cluster %d to 0x%x\n", cluster, value);
	return true;
};

/**
 * Creates a new file in the FAT16 file system.
 * Steps:
 * 1️⃣ Validate parameters (`dev`, `path`), return error if invalid.
 * 2️⃣ Find a free directory entry in the parent directory.
 * 3️⃣ Locate a free cluster in the FAT table.
 * 4️⃣ Prepare the file entry (name, attributes, start cluster, size = 0).
 * 5️⃣ Update the FAT table (mark cluster as allocated or EOF).
 * 6️⃣ Write the file entry into the parent directory.
 * 7️⃣ Return a `fat16_node_t` structure representing the new file.
 */
static fat16_node_t* _create_fat16_file(ata_t* dev, pathnode_t* path)
{
	if (!dev || !path) {
		return 0x0;
	};
	pathnode_t* curr = path;
	// Step 2: Check if the parent directory is the root
	const bool is_root_dir = !curr->next;
	pathnode_t* parent = curr;
	// Step 3: Get the starting cluster or fixed root directory area
	const uint32_t partition_offset = 0x100000;
	const uint32_t parent_cluster = is_root_dir ? _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset) : _get_dir_cluster(dev, parent);
	// Step 4: If the parent directory does not exist, abort file creation
	if (!parent_cluster) {
		printf("[FAT16] ERROR: Parent directory '%s' not found. File creation failed.\n", is_root_dir ? "/" : parent->identifier);
		return 0x0;
	};
	printf("[FAT16] INFO: Creating File in %s (Cluster: %d)\n", is_root_dir ? "Root Directory" : parent->identifier, parent_cluster);
	// Step 5: Find a free directory entry
	const uint32_t free_entry = is_root_dir ? _find_free_root_dir_entry(dev) : _get_free_dir_entry(dev, parent_cluster);

	if (!free_entry) {
		printf("[FAT16] ERROR: No free Space in Directory for '%s'\n", path->identifier);
		return 0x0;
	};
	// Step 6: Find a free cluster for the file's data storage
	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	const uint32_t free_cluster = _find_free_cluster(&fat_stream, partition_offset);

	if (!free_cluster) {
		printf("[FAT16] ERROR: No free clusters available. File creation failed.\n");
		return 0x0;
	};
	printf("[FAT16] INFO: Allocated Cluster %d for new File '%s'\n", free_cluster, path->identifier);
	// Step 7: Prepare the file entry
	fat16_dir_entry_t new_entry = {};
	memset(&new_entry, 0, sizeof(fat16_dir_entry_t));
	_convert_userland_filename_to_native(new_entry.file_name, (uint8_t*)path->identifier);
	new_entry.attributes = ARCHIVE;
	new_entry.high_cluster = (free_cluster >> 16) & 0xFFFF;
	new_entry.low_cluster = free_cluster & 0xFFFF;
	new_entry.file_size = 0;
	// _dump_fat_table(dev);
	// Step 8: Mark the allocated cluster in the FAT as EOF (end of file)
	_update_fat_entry(dev, free_cluster, FAT16_VALUE_END_OF_CHAIN);
	// Step 9: Write the file entry into the directory
	if (is_root_dir) {
		_write_root_dir_entry(dev, free_entry, &new_entry);
	} else {
		_write_dir_entry(dev, parent_cluster, &new_entry);
	};
	// Step 10: Return a FAT16 node representing the new file
	fat16_node_t* fat16_entry = kzalloc(sizeof(fat16_node_t));
	fat16_entry->file = kzalloc(sizeof(fat16_dir_entry_t));
	memcpy(fat16_entry->file, &new_entry, sizeof(fat16_dir_entry_t));
	fat16_entry->type = FAT16_ENTRY_TYPE_FILE;
	// Convert the native fat16 filename to more userland friendly 'BLA     .BIN' to 'BLA.BIN'
	char userland[11];
	_convert_fat16_native_to_userland(userland, fat16_entry->file->file_name);
	printf("[FAT16] SUCCESS: Created file '%s' in '%s'\n", userland, is_root_dir ? "/" : parent->identifier);
	return fat16_entry;
};

size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks)
{
	if (!dev || !internal || !buffer || n_bytes == 0 || n_blocks == 0) {
		return -EINVAL;
	};
	fat16_fd_t* fd = (fat16_fd_t*)internal;
	fat16_dir_entry_t* file_entry = fd->entry->file;
	uint16_t curr_cluster = _combine_clusters(file_entry->high_cluster, file_entry->low_cluster);
	size_t bytes_written = 0;
	size_t bytes_to_write = n_bytes * n_blocks;
	return 0;
};

void fat16_dump_root_dir_entry_at(ata_t* dev, const int32_t i)
{
	if (!dev) {
		return;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_sector = _calc_root_dir_area_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);

	uint8_t buffer[root_dir_size];

	stream_t stream = {};
	stream_init(&stream, dev);
	stream_seek(&stream, root_dir_sector);

	const int32_t res = stream_read(&stream, buffer, root_dir_size);

	if (res < 0) {
		return;
	};
	fat16_dir_entry_t* entry = (fat16_dir_entry_t*)(buffer + (i * sizeof(fat16_dir_entry_t)));
	_print_dir_entry(i, entry, 0);
	return;
};