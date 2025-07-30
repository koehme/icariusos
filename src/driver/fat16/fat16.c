/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-13
 */

#include "fat16.h"
#include "cmos.h"
#include "icarius.h"
#include "stream.h"
#include "string.h"

/* PUBLIC API */
fat16_time_t fat16_get_curr_time(void);
uint16_t fat16_get_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster);
void fat16_dump_fat_entry(ata_t* dev, uint16_t cluster);
uint32_t fat16_find_free_entry_in_dir(ata_t* dev, uint16_t start_cluster);
fat16_node_t* fat16_create_file(ata_t* dev, pathnode_t* path, fat16_node_t* node);
bool fat16_create_fat_entry(ata_t* dev, uint32_t cluster, uint16_t value);
bool fat16_create_root_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry);
bool fat16_create_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry);
fat16_node_t* fat16_create_node_from_entry(fat16_dir_entry_t* entry, const uint32_t parent_sector_start);
fat16_node_t* fat16_create_node_from_dir(fat16_folder_t* folder, fat16_dir_entry_t* entry);
void fat16_create_file_stat(fat16_fd_t* fat16_descriptor, ata_t* dev, vstat_t* vstat, uint32_t max_cluster_size_bytes, uint32_t used_blocks);
uint32_t fat16_get_cluster_from_path(ata_t* dev, pathnode_t* path);
uint32_t fat16_get_free_root_dir_entry(ata_t* dev);
uint16_t fat16_get_next_free_cluster(stream_t* fat_stream, const uint32_t partition_offset);
void fat16_get_native_filename_from_userland(uint8_t* native, const uint8_t* userland);
void fat16_get_userland_filename_from_native(char* userland, const uint8_t* native);
uint32_t fat16_get_root_dir_offset(const bpb_t* bpb);
uint32_t fat16_get_root_dir_absolute(const bpb_t* bpb, const uint32_t partition_offset);
uint32_t fat16_get_fat_table_offset(const bpb_t* bpb);
uint32_t fat16_get_fat_table_absolute(const bpb_t* bpb, const uint32_t partition_offset);
fat16_time_t fat16_get_unpacked_time(const uint16_t time);
fat16_date_t fat16_get_unpacked_date(const uint16_t date);
uint16_t fat16_get_packed_date(const fat16_date_t date);
uint32_t fat16_get_combined_cluster(const uint16_t high_cluster, const uint16_t low_cluster);
fat16_dir_entry_t* fat16_get_root_dir_entry(ata_t* dev, pathnode_t* path, fat16_dir_entry_t* entry, fat16_folder_t* folder);
uint32_t fat16_get_sector_from_cluster(const uint32_t cluster);
uint16_t fat16_get_cluster_chain_length(stream_t* fat_stream, const uint16_t max_cluster_size_bytes, const uint32_t start_cluster,
					const uint32_t partition_offset);
void fat16_dump_ebpb_header(const ebpb_t* ebpb, const char* msg, const int32_t delay);
void fat16_dump_base_header(const bpb_t* bpb, const char* msg, const int32_t delay);
void fat_16_dump_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
void fat16_dump_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
void fat16_dump_root_dir(const bpb_t* bpb, stream_t* stream);
void fat16_dump_fat_table(ata_t* dev);
void fat16_dump_root_dir_entry_at(ata_t* dev, const int32_t i);
fat16_dir_entry_t* fat16_get_node_from_path_in_cluster_chain(ata_t* dev, const uint16_t start_cluster, const uint8_t* fat16_filename,
							     fat16_folder_t* fat16_folder, fat16_dir_entry_t* fat16_dir_entry);
int32_t fat16_resolve(ata_t* dev);
fat16_node_t* fat16_get_node_from_path(ata_t* dev, pathnode_t* path);
void* fat16_open(ata_t* dev, pathnode_t* path, uint8_t mode);
uint32_t fat16_get_cluster_from_descriptor(fat16_fd_t* fat16_descriptor);
size_t fat16_read(ata_t* dev, void* descriptor, uint8_t* buffer, const size_t n_bytes, const size_t n_blocks);
int32_t fat16_close(void* internal);
int32_t fat16_stat(ata_t* dev, void* internal, vstat_t* vstat);
int32_t fat16_seek(void* internal, const uint32_t offset, const uint8_t origin);
size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks);
int32_t fat16_readdir(ata_t* dev, void* internal, vfs_dirent_t* dir, uint32_t dir_offset);
/* INTERNAL API */
bool _is_valid_fat16_header(const fat16_internal_header_t* header);

fs_t fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .read_cb = 0x0,
    .close_cb = 0x0,
    .stat_cb = 0x0,
    .seek_cb = 0x0,
    .readdir_cb = 0x0,
    .name = "FAT16",
};

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

fat16_date_t fat16_get_curr_date(void)
{
	const date_t date = cmos_date(&cmos);

	fat16_date_t fat16_date = {
	    .day = date.day,
	    .month = date.month,
	    .year = date.year,
	};
	return fat16_date;
};

fat16_time_t fat16_get_curr_time(void)
{
	const time_t time = cmos_time(&cmos);

	fat16_time_t fat16_time = {
	    .hour = time.hour,
	    .minute = time.minute,
	    .second = time.second,
	};
	return fat16_time;
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
	fat16.readdir_cb = fat16_readdir;
	return &fat16;
};

uint16_t fat16_get_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster)
{
	uint8_t fat_entry[2] = {};
	const uint32_t fat_entry_offset = partition_offset + fat16_header.bpb.rsvd_sec * fat16_header.bpb.byts_per_sec + curr_cluster * 2;
	stream_seek(fat_stream, fat_entry_offset);
	stream_read(fat_stream, fat_entry, 2 * sizeof(uint8_t));
	return ((uint16_t)fat_entry[0]) | ((uint16_t)fat_entry[1] << 8);
};

void fat16_dump_fat_entry(ata_t* dev, uint16_t cluster)
{
	if (!dev || cluster < 2 || cluster >= FAT16_MAX_CLUSTERS) {
		printf("[FAT16] ERROR: Invalid device or cluster %d\n", cluster);
		return;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t fat_table_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_entry_offset = fat_table_offset + (cluster * 2);

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	stream_seek(&fat_stream, fat_entry_offset);

	uint8_t fat_entry[2] = {0};

	if (stream_read(&fat_stream, fat_entry, sizeof(fat_entry)) < 0) {
		printf("[FAT16] ERROR: Failed to read FAT entry for cluster %d\n", cluster);
		return;
	};
	uint16_t value = fat_entry[0] | (fat_entry[1] << 8);
	printf("\n[FAT16] FAT Entry for Cluster %d: 0x%x\n", cluster, value);

	if (value == FAT16_VALUE_FREE) {
		printf("\t- Cluster is free\n");
	} else if (value >= FAT16_VALUE_END_OF_CHAIN) {
		printf("\t- End of chain (EOF)\n");
	} else if (value == FAT16_VALUE_BAD_CLUSTER) {
		printf("\t- Bad cluster\n");
	} else {
		printf("\t- Next cluster in chain: %d\n", value);
	};
	return;
};

uint32_t fat16_find_free_entry_in_dir(ata_t* dev, uint16_t start_cluster)
{
	if (!dev || start_cluster < 2 || start_cluster >= FAT16_MAX_CLUSTERS) {
		printf("[FAT16] ERROR: Invalid parameters or start_cluster %d\n", start_cluster);
		return 0;
	};
	const uint32_t partition_offset = 0x100000;
	stream_t fat_stream = {}, data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);

	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
	uint8_t buffer[max_cluster_size_bytes];
	uint16_t curr_cluster = start_cluster;

	const uint32_t num_entries_per_cluster = max_cluster_size_bytes / sizeof(fat16_dir_entry_t);

	while (curr_cluster < FAT16_VALUE_END_OF_CHAIN) {
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		if (stream_read(&data_stream, buffer, max_cluster_size_bytes) < 0) {
			printf("[FAT16] ERROR: Failed reading Cluster %d\n", curr_cluster);
			return 0;
		};
		fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;

		for (uint32_t i = 0; i < num_entries_per_cluster; i++, entry++) {
			if (entry->file_name[0] == FREE || entry->file_name[0] == DELETED) {
				printf("[FAT16] INFO: Found free Entry at cluster %d, Entry %d\n", curr_cluster, i);
				return data_pos + (i * sizeof(fat16_dir_entry_t));
			};
		};
		// Next cluster in the chain
		curr_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	// No slot found, create new cluster
	const uint16_t new_cluster = fat16_get_next_free_cluster(&fat_stream, partition_offset);

	if (!new_cluster) {
		printf("[FAT16] ERROR: No free clusters available.\n");
		return 0;
	};
	// Append cluster to fat table
	fat16_create_fat_entry(dev, curr_cluster, new_cluster);
	fat16_create_fat_entry(dev, new_cluster, FAT16_VALUE_END_OF_CHAIN);
	// Init new cluster
	const uint32_t new_sector = fat16_get_sector_from_cluster(new_cluster);
	const uint32_t new_data_pos = partition_offset + (new_sector * fat16_header.bpb.byts_per_sec);
	memset(buffer, 0, max_cluster_size_bytes);
	stream_seek(&data_stream, new_data_pos);

	if (stream_write(&data_stream, buffer, max_cluster_size_bytes) < 0) {
		printf("[FAT16] ERROR: Failed initializing new cluster %d\n", new_cluster);
		return -EIO;
	};
	printf("[FAT16] INFO: Created and initialized new cluster %d\n", new_cluster);
	return new_data_pos;
};

// Creates a new file in the FAT16 file system
fat16_node_t* fat16_create_file(ata_t* dev, pathnode_t* path, fat16_node_t* node)
{
	if (!dev || !path) {
		return 0x0;
	};
	const uint32_t partition_offset = 0x100000;
	pathnode_t* curr = path;

	while (curr && curr->next) {
		curr = curr->next;
	};
	uint32_t free_entry_offset = 0;

	if (!path->next) {
		// Use Root dir to find a free entry
		free_entry_offset = fat16_get_free_root_dir_entry(dev);

		if (!free_entry_offset) {
			printf("[FAT16] ERROR: Root Directory FULL!\n");
			return 0x0;
		};
	} else {
		// Use FAT CHAIN to find a free entry
		printf("[FAT16] Creating File %s in Folder '%s'\n", curr->identifier, path->identifier);
		const uint32_t cluster = fat16_get_combined_cluster(node->dir->entry->high_cluster, node->dir->entry->low_cluster);
		free_entry_offset = fat16_find_free_entry_in_dir(dev, cluster);
	};
	// BUILD NEW FAT ENTRY
	fat16_dir_entry_t new_entry = {};
	memset(&new_entry, 0, sizeof(fat16_dir_entry_t));
	fat16_get_native_filename_from_userland(new_entry.file_name, (uint8_t*)curr->identifier);
	new_entry.attributes = ARCHIVE;
	// ALLOCATE NEW DATA CLUSTER FOR NEW FAT ENTRY
	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	const uint32_t fat_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	stream_seek(&fat_stream, fat_offset);
	const uint32_t free_cluster = fat16_get_next_free_cluster(&fat_stream, partition_offset);

	if (!free_cluster) {
		printf("[FAT16] ERROR: No free clusters available.\n");
		return 0x0;
	};
	new_entry.high_cluster = (free_cluster >> 16) & 0xFFFF;
	new_entry.low_cluster = free_cluster & 0xFFFF;
	new_entry.file_size = 0;
	printf("[FAT16] INFO: Allocated Cluster %d for new File '%s'\n", free_cluster, curr);
	// Mark the cluster from the new entry itself as end of chain
	fat16_create_fat_entry(dev, free_cluster, FAT16_VALUE_END_OF_CHAIN);

	stream_t dir_stream = {};
	stream_init(&dir_stream, dev);
	stream_seek(&dir_stream, free_entry_offset);

	if (stream_write(&dir_stream, (uint8_t*)&new_entry, sizeof(fat16_dir_entry_t) < 0)) {
		printf("[FAT16] ERROR: Failed Writing.\n");
		return 0x0;
	};
	printf("[FAT16] SUCCESS: File '%s' created at Offset 0x%x (Cluster: %d)\n", curr->identifier, free_entry_offset, free_cluster);

	// Create new fat16 node
	fat16_node_t* fat16_entry = kzalloc(sizeof(fat16_node_t));
	fat16_entry->file = kzalloc(sizeof(fat16_dir_entry_t));
	memcpy(fat16_entry->file, &new_entry, sizeof(fat16_dir_entry_t));
	fat16_entry->type = FAT16_ENTRY_TYPE_FILE;

	// Convert fat 16 filename to userland
	char userland[11];
	fat16_get_userland_filename_from_native(userland, fat16_entry->file->file_name);

	printf("[FAT16] SUCCESS: Created file '%s' in '%s'\n", userland, path->identifier);
	return fat16_entry;
};

bool fat16_create_fat_entry(ata_t* dev, uint32_t cluster, uint16_t value)
{
	if (!dev || cluster < 2 || cluster >= FAT16_MAX_CLUSTERS) {
		return false;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t fat_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_entry_offset = fat_offset + cluster * 2;

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	stream_seek(&fat_stream, fat_entry_offset);

	uint8_t fat_entry[2] = {(uint8_t)(value & 0xFF), (uint8_t)((value >> 8) & 0xFF)};
	const int32_t res = stream_write(&fat_stream, (uint8_t*)fat_entry, sizeof(uint16_t));

	if (res < 0) {
		printf("[FAT16] ERROR: Failed to update FAT Entry for Cluster %d\n", cluster);
		return false;
	};
	printf("[FAT16] SUCCESS: Updated FAT Entry for Cluster %d to 0x%x\n", cluster, value);
	return true;
};

bool fat16_create_root_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry)
{
	if (!dev || !new_entry) {
		return false;
	};
	const uint32_t entry_offset = entry_sector;
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

fat16_node_t* fat16_create_node_from_entry(fat16_dir_entry_t* entry, uint32_t parent_sector_start)
{
	if (!entry) {
		return 0x0;
	};
	fat16_node_t* node = kzalloc(sizeof(fat16_node_t));

	if (!node) {
		return 0x0;
	};
	node->file = kzalloc(sizeof(fat16_dir_entry_t));

	if (!node->file) {
		kfree(node);
		return 0x0;
	};
	node->type = FAT16_ENTRY_TYPE_FILE;
	memcpy(node->file, entry, sizeof(fat16_dir_entry_t));
	return node;
};

bool fat16_update_dir_entry(ata_t* dev, fat16_dir_entry_t* update, char filename[11]) { return true; };

fat16_node_t* fat16_create_node_from_dir(fat16_folder_t* folder, fat16_dir_entry_t* entry)
{
	if (!folder || !entry) {
		return 0x0;
	};
	fat16_node_t* node = kzalloc(sizeof(fat16_node_t));

	if (!node) {
		return 0x0;
	};
	node->dir = kzalloc(sizeof(fat16_folder_t));

	if (!node->dir) {
		kfree(node);
		return 0x0;
	};
	memcpy(node->dir, folder, sizeof(fat16_folder_t));
	node->dir->entry = kzalloc(sizeof(fat16_dir_entry_t));

	if (!node->dir->entry) {
		kfree(node->dir);
		kfree(node);
		return 0x0;
	};
	memcpy(node->dir->entry, entry, sizeof(fat16_dir_entry_t));
	node->type = FAT16_ENTRY_TYPE_DIRECTORY;
	return node;
};

void fat16_create_file_stat(fat16_fd_t* fat16_descriptor, ata_t* dev, vstat_t* vstat, uint32_t max_cluster_size_bytes, uint32_t used_blocks)
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

uint32_t fat16_get_cluster_from_path(ata_t* dev, pathnode_t* path)
{
	if (!dev || !path) {
		return 0;
	};
	const uint32_t partition_offset = 0x100000;
	uint32_t curr_cluster = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
	pathnode_t* curr = path;

	while (curr) {
		fat16_folder_t parent_dir = {};
		fat16_dir_entry_t dir_entry = {};

		if (!fat16_get_node_from_path_in_cluster_chain(dev, curr_cluster, (uint8_t*)curr->identifier, &parent_dir, &dir_entry)) {
			printf("[FAT16] ERROR: Directory '%s' not found!\n", curr->identifier);
			return 0;
		};
		curr_cluster = fat16_get_combined_cluster(dir_entry.high_cluster, dir_entry.low_cluster);

		if (curr_cluster == 0) {
			printf("[FAT16] ERROR: Cluster Calculation failed for '%s'\n", curr->identifier);
			return 0;
		};
		curr = curr->next;
	};
	return curr_cluster;
};

uint32_t fat16_get_free_root_dir_entry(ata_t* dev)
{
	if (!dev) {
		return 0x0;
	};
	// Calculate the position of the root directory area
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_sector = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
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

uint16_t fat16_get_next_free_cluster(stream_t* fat_stream, const uint32_t partition_offset)
{
	uint8_t fat_entry[2] = {};

	for (uint16_t cluster = 3; cluster < FAT16_MAX_CLUSTERS; cluster++) {
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

void fat16_get_native_filename_from_userland(uint8_t* native, const uint8_t* userland)
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

void fat16_get_userland_filename_from_native(char* userland, const uint8_t* native)
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

uint32_t fat16_get_root_dir_offset(const bpb_t* bpb)
{
	const uint32_t root_directory_offset = bpb->byts_per_sec * (bpb->rsvd_sec + (bpb->num_fats * bpb->fatsz16));
	return root_directory_offset;
};

uint32_t fat16_get_root_dir_absolute(const bpb_t* bpb, const uint32_t partition_offset)

{
	const uint32_t root_dir_area_absolute = partition_offset + fat16_get_root_dir_offset(bpb);
	return root_dir_area_absolute;
};

uint32_t fat16_get_fat_table_offset(const bpb_t* bpb)
{
	const uint32_t fat_area_offset = bpb->rsvd_sec * bpb->byts_per_sec;
	return fat_area_offset;
};

uint32_t fat16_get_fat_table_absolute(const bpb_t* bpb, const uint32_t partition_offset)
{
	const uint32_t fat_area_absolute = partition_offset + fat16_get_fat_table_offset(bpb);
	return fat_area_absolute;
};

fat16_time_t fat16_get_unpacked_time(const uint16_t time)
{
	fat16_time_t time_info = {};
	time_info.second = (time & 0x1F) * 2;
	time_info.minute = (time >> 5) & 0x3F;
	time_info.hour = (time >> 11) & 0x1F;
	return time_info;
};

fat16_date_t fat16_get_unpacked_date(const uint16_t date)
{
	fat16_date_t date_info = {};
	date_info.day = date & 0x1F;
	date_info.month = (date >> 5) & 0x0F;
	const uint16_t original_year = (date >> 9) & 0x7F;
	date_info.year = (original_year == 0) ? 0 : (original_year + 1980);
	return date_info;
};

uint16_t fat16_get_packed_date(const fat16_date_t date)
{
	uint16_t packed_date = 0;
	packed_date |= ((date.year - 1980) & 0x7F) << 9;
	packed_date |= (date.month & 0x0F) << 5;
	packed_date |= (date.day & 0x1F);
	return packed_date;
};

uint16_t fat16_get_packed_time(fat16_time_t time) { return ((time.hour & 0x1F) << 11) | ((time.minute & 0x3F) << 5) | ((time.second / 2) & 0x1F); };

uint32_t fat16_get_combined_cluster(const uint16_t high_cluster, const uint16_t low_cluster)
{
	const uint32_t cluster = (high_cluster << 16) | low_cluster;
	return cluster;
};

// Searches for a specific file or directory in the FAT16 root directory to locate its entry for further access or manipulation
fat16_dir_entry_t* fat16_get_root_dir_entry(ata_t* dev, pathnode_t* path, fat16_dir_entry_t* entry, fat16_folder_t* folder)
{
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_area_absolute = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);

	stream_t stream_t = {};
	stream_init(&stream_t, dev);
	stream_seek(&stream_t, root_dir_area_absolute);

	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
	const uint32_t first_data_sector = fat16_header.bpb.rsvd_sec + fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16 + (root_dir_sectors);
	const uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
	const uint32_t last_root_dir_sector = first_root_dir_sector + root_dir_sectors;
	const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	uint8_t buffer[root_dir_size];

	const int32_t res = stream_read(&stream_t, buffer, root_dir_size);

	if (res < 0) {
		printf("[Stream] ERROR: Failed to read Root Dir (ATA Error %d)\n", res);
		return 0x0;
	};

	if (!path || strcmp(path->identifier, "/") == 0) {
		folder->start_sector = first_root_dir_sector;
		folder->end_sector = last_root_dir_sector;
		folder->total = fat16_header.bpb.root_ent_cnt;
		folder->entry = 0x0;
		return 0x0;
	};
	fat16_dir_entry_t* curr_root_entry = (fat16_dir_entry_t*)buffer;

	for (size_t i = 0; i < fat16_header.bpb.root_ent_cnt; i++, curr_root_entry++) {
		if (curr_root_entry->file_name[0] != 0x0) {
			uint8_t native_dir_name[11] = {};
			fat16_get_native_filename_from_userland(native_dir_name, (uint8_t*)path);

			if (memcmp(curr_root_entry->file_name, native_dir_name, 11) == 0) {
				memcpy(entry, curr_root_entry, sizeof(fat16_dir_entry_t));
				folder->start_sector = first_root_dir_sector + (i * sizeof(fat16_dir_entry_t)) / fat16_header.bpb.byts_per_sec;
				folder->end_sector = folder->start_sector + ((i + 1) * sizeof(fat16_dir_entry_t)) / fat16_header.bpb.byts_per_sec;
				folder->total = fat16_header.bpb.root_ent_cnt;
				break;
			};
		};
	};
	return entry;
};

uint32_t fat16_get_sector_from_cluster(const uint32_t cluster)
{
	// Calculate the number of sectors occupied by the root directory and ensures that we round up if the root directory size is not a multiple of the
	// sector size
	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec; // 32
	// Compute the first sector of the data region
	const uint32_t data_start_sector = fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16) + root_dir_sectors; // 560
	// FAT16 cluster numbering starts at 2 for data clusters. Cluster 0 and 1 are reserved and not used for storing data. Subtracting 2 aligns the cluster
	// number with the correct sector offset
	return data_start_sector + ((cluster - 2) * fat16_header.bpb.sec_per_clus);
};

uint16_t fat16_get_cluster_chain_length(stream_t* fat_stream, const uint16_t max_cluster_size_bytes, const uint32_t start_cluster,
					const uint32_t partition_offset)
{
	uint16_t used_blocks = 1;

	uint16_t curr_cluster = (0 / max_cluster_size_bytes) + start_cluster;
	uint16_t next_cluster = fat16_get_next_cluster(fat_stream, partition_offset, curr_cluster);

	while (next_cluster < FAT16_VALUE_END_OF_CHAIN) {
		used_blocks++;
		curr_cluster = next_cluster;
		next_cluster = fat16_get_next_cluster(fat_stream, partition_offset, curr_cluster);
	};
	return used_blocks;
};

void fat16_dump_ebpb_header(const ebpb_t* ebpb, const char* msg, const int32_t delay)
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

void fat16_dump_base_header(const bpb_t* bpb, const char* msg, const int32_t delay)
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

void fat_16_dump_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay)
{
	const fat16_time_t create_time = fat16_get_unpacked_time(entry->create_time);
	const fat16_date_t create_date = fat16_get_unpacked_date(entry->create_date);
	const fat16_date_t last_access_date = fat16_get_unpacked_date(entry->last_access_date);
	const fat16_time_t mod_time = fat16_get_unpacked_time(entry->modification_time);
	const fat16_date_t mod_date = fat16_get_unpacked_date(entry->modification_date);

	uint8_t buffer[11] = {};
	memcpy(buffer, entry->file_name, 11);

	printf("==========================\n");
	printf("=   Fat16DirEntry %d:\n", i);
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
	return;
};

void fat16_dump_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay)
{
	if (entry->file_name[0] == 0x00) {
		return;
	};
	printf("==========================\n");
	printf("=   Fat16DirLFNEntry %d:\n", i);
	printf("==========================\n");
	printf("=   LFN: %s (Long File Name NOT SUPPORTED)\n", "-");
	printf("==========================\n");
	return;
};

void fat16_dump_root_dir(const bpb_t* bpb, stream_t* stream)
{
	const uint32_t root_dir_size = bpb->root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t root_dir_entries = root_dir_size / sizeof(fat16_dir_entry_t);

	for (size_t i = 0; i < bpb->root_ent_cnt; i++) {
		fat16_dir_entry_t entry = {};
		const int32_t res = stream_read(stream, (uint8_t*)&entry, sizeof(fat16_dir_entry_t));

		if (res != 0) {
			printf("FAT16 Error: Failed to read FAT16 Header\n");
			return;
		};

		if (entry.attributes == LFN) {
			fat16_dump_lfn_entry(i, &entry, FAT16_DEBUG_DELAY);
			continue;
		};
		if (entry.file_name[0] != 0) {
			fat_16_dump_entry(i, &entry, FAT16_DEBUG_DELAY);
		};
	};
	return;
};

void fat16_dump_fat_table(ata_t* dev)
{
	if (!dev) {
		printf("[FAT16] ERROR: Invalid Dev\n");
		return;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t fat_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
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
			printf("Cluster %d: End of chain (EOF)\n", cluster);
		} else if (fat_entry == FAT16_VALUE_BAD_CLUSTER) {
			printf("Cluster %d: Bad cluster\n", cluster);
		} else if (fat_entry == FAT16_VALUE_FREE) {
			printf("Cluster %d: Free cluster\n", cluster);
		};
	};
	return;
};

void fat16_dump_root_dir_entry_at(ata_t* dev, const int32_t i)
{
	if (!dev) {
		return;
	};
	const uint32_t partition_offset = 0x100000;
	const uint32_t root_dir_sector = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
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
	fat_16_dump_entry(i, entry, 0);
	return;
};

fat16_dir_entry_t* fat16_get_node_from_path_in_cluster_chain(ata_t* dev, const uint16_t start_cluster, const uint8_t* fat16_filename,
							     fat16_folder_t* fat16_folder, fat16_dir_entry_t* fat16_dir_entry)
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
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		const int32_t res = stream_read(&data_stream, buffer, max_cluster_size_bytes);

		if (res < 0) {
			printf("[Stream] ERROR: Failed to read Cluster %d (ATA Error %d)\n", curr_cluster, res);
			return 0x0;
		};
		uint8_t* curr_dir_entry = buffer;

		for (int i = 0; i < num_entries; i++) {
			char buf[11] = {};
			memcpy(buf, ((fat16_dir_entry_t*)curr_dir_entry)->file_name, 11);

			if (memcmp(buf, fat16_filename, 11) == 0) {
				const uint32_t start_sector = fat16_get_sector_from_cluster(start_cluster);
				const uint32_t end_sector = fat16_get_sector_from_cluster(curr_cluster);
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
		curr_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	fat16_folder->total = total_entries;
	return 0x0;
};

bool _is_valid_fat16_header(const fat16_internal_header_t* header)
{
	if (!header) {
		return false;
	};
	const bool has_signature = (header->bpb.jmp[0] == FAT16_BOOT_SIG_1 && header->bpb.jmp[2] == FAT16_BOOT_SIG_2);
	const bool has_header = has_signature && (header->bpb.byts_per_sec == FAT16_SECTOR_SIZE) && (header->bpb.sec_per_clus >= 1) &&
				(header->bpb.num_fats == 2) && (header->bpb.media != 0);

	if (!has_header) {
		return false;
	};
	const bool has_ebpb = (header->ebpb.boot_sig == FAT16_EBPB_SIG) && (header->ebpb.drv_num == FAT16_HARD_DISK);

	if (has_ebpb) {
		const uint32_t total_sectors = (header->bpb.tot_sec_32 != 0) ? header->bpb.tot_sec_32 : header->bpb.tot_sec_16;
		const uint32_t disk_size = total_sectors * header->bpb.byts_per_sec;

		if (!disk_size || !total_sectors) {
			return false;
		};
	};
	return true;
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

	if (!_is_valid_fat16_header(&fat16_header)) {
		printf("FAT16 Error: Invalid FAT16 Header\n");
		return -EIO;
	};
	// fat16_dump_base_header(&fat16_header.bpb, "", 0);
	// fat16_dump_ebpb_header(&fat16_header.ebpb, "", 0);
	const uint32_t root_dir_area_offset = fat16_get_root_dir_offset(&fat16_header.bpb);
	const uint32_t root_dir_area_absolute = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t root_dir_area_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t root_dir_area_entries = root_dir_area_size / sizeof(fat16_dir_entry_t);

	printf("Root Dir Area Offset: 0x%x\n", root_dir_area_offset);
	printf("Root Dir Area Absolute: 0x%x\n", root_dir_area_absolute);
	printf("Root Dir Area Size: %d\n", root_dir_area_size);
	printf("Root Dir Area Entries: %d\n", root_dir_area_entries);

	stream_t root_dir = {};
	stream_init(&root_dir, dev);
	stream_seek(&root_dir, root_dir_area_absolute);

	const uint32_t fat_area_offset = fat16_get_fat_table_offset(&fat16_header.bpb);
	const uint32_t fat_area_absolute = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	const uint32_t fat_area_size = fat16_header.bpb.fatsz16 * fat16_header.bpb.byts_per_sec;
	const uint32_t fat_area_entries = fat_area_size / sizeof(uint16_t);

	printf("FAT Area Offset: 0x%x\n", fat_area_offset);
	printf("FAT Area Absolute: 0x%x\n", fat_area_absolute);
	printf("FAT Area Size: %d\n", fat_area_size);
	printf("FAT Area Entries: %d\n", fat_area_entries);

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

fat16_node_t* fat16_get_node_from_path(ata_t* dev, pathnode_t* path)
{
	if (!dev || !path) {
		return 0x0;
	};
	pathnode_t* curr = path;

	while (curr && curr->next) {
		curr = curr->next;
	};
	uint8_t native[11] = {};
	fat16_get_native_filename_from_userland(native, (uint8_t*)curr);

	fat16_dir_entry_t entry = {};
	fat16_folder_t folder = {};
	fat16_get_root_dir_entry(dev, path, &entry, &folder);
	// Root Dir '/' handler
	if (!folder.entry && (folder.end_sector - folder.start_sector) == 32) {
		return fat16_create_node_from_dir(&folder, &entry);
	};

	if (!path->next) {
		if (entry.file_name[0] && (entry.attributes & DIRECTORY)) {
			return fat16_create_node_from_dir(&folder, &entry);
		};

		if (entry.file_name[0] && (entry.attributes & ARCHIVE)) {
			return fat16_create_node_from_entry(&entry, folder.start_sector);
		};
	};
	const uint32_t root_dir_entry_cluster = fat16_get_combined_cluster(entry.high_cluster, entry.low_cluster);
	const uint32_t start_cluster = root_dir_entry_cluster;
	const uint32_t parent_sector = folder.start_sector;
	fat16_dir_entry_t* found_entry = fat16_get_node_from_path_in_cluster_chain(dev, start_cluster, native, &folder, &entry);

	if (!found_entry) {
		printf("Error: Folder '%s' not found!\n", curr->identifier);
		return 0x0;
	};

	if (entry.file_name[0] && (entry.attributes & DIRECTORY)) {
		folder.entry = &entry;
		return fat16_create_node_from_dir(&folder, &entry);
	};

	if (entry.file_name[0] && (entry.attributes & ARCHIVE)) {
		return fat16_create_node_from_entry(&entry, parent_sector);
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
	fat16_node_t* entry = fat16_get_node_from_path(dev, path);

	if (!entry && mode == READ) {
		printf("[FAT16] ERROR: File '%s' not found for reading\n", path->identifier);
		return 0x0;
	};

	if (entry->type == FAT16_ENTRY_TYPE_DIRECTORY && entry->dir->entry && mode == WRITE) {
		printf("[FAT16] File not found - Creating FAT16 Entry in Parent Folder %s\n", entry->dir->entry->file_name);
		entry = fat16_create_file(dev, path, entry);

		if (!entry) {
			printf("[FAT16] ERROR: File Creation failed\n");
			return 0x0;
		};
	};
	fat16_fd_t* fd = kzalloc(sizeof(fat16_fd_t));

	if (!fd) {
		printf("[FAT16] Memory Allocation failed\n");
		return 0x0;
	};
	fd->entry = entry;
	fd->pos = 0;
	return fd;
};

uint32_t fat16_get_cluster_from_descriptor(fat16_fd_t* fat16_descriptor)
{
	uint32_t start_cluster = 0;

	switch (fat16_descriptor->entry->type) {
	case FAT16_ENTRY_TYPE_DIRECTORY: {
		start_cluster = fat16_get_combined_cluster(fat16_descriptor->entry->dir->entry->high_cluster, fat16_descriptor->entry->dir->entry->low_cluster);
		break;
	};
	case FAT16_ENTRY_TYPE_FILE: {
		start_cluster = fat16_get_combined_cluster(fat16_descriptor->entry->file->high_cluster, fat16_descriptor->entry->file->low_cluster);
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

	if (fat16_descriptor->entry->type == FAT16_ENTRY_TYPE_FILE && fat16_descriptor->pos >= fat16_descriptor->entry->file->file_size) {
		return 0;
	};
	// Maximum cluster size in bytes of a single FAT16 cluster
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
	// The given file descriptor holds a pointer to the root dir entry, which holds the fat16 start cluster number to read
	const uint32_t start_cluster = fat16_get_cluster_from_descriptor(fat16_descriptor);
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
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		// Use the previously calculated sector and add the partition offset and the first cluster to get the data position on die ata device
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec) + first_cluster_offset;
		// Seek to the data position and read data into the buffer
		stream_seek(&data_stream, data_pos);
		// Ensure that new data is appended to the buffer + bytes_read ensures the correct position so that previously written data is not
		// overwritten
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
		const uint16_t next_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
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
	const uint32_t start_cluster = fat16_get_cluster_from_descriptor(fat16_descriptor);
	// Follow the cluster chain from the start 0 from the starting cluster and keep track of allocated blocks
	const uint16_t used_blocks = fat16_get_cluster_chain_length(&fat_stream, max_cluster_size_bytes, start_cluster, partition_offset);
	fat16_create_file_stat(fat16_descriptor, dev, vstat, max_cluster_size_bytes, used_blocks);
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

void fat16_free_unused_clusters(ata_t* dev, stream_t* fat_stream, uint16_t start_cluster, size_t required_clusters, size_t existing_clusters,
				uint32_t fat_table_offset)
{
	if (!dev || !fat_stream || required_clusters >= existing_clusters || start_cluster < 2) {
		return;
	};
	uint16_t curr_cluster = start_cluster;
	uint16_t prev_cluster = 0;
	size_t cluster_count = 1;

	while (cluster_count <= required_clusters && curr_cluster < FAT16_VALUE_END_OF_CHAIN) {
		prev_cluster = curr_cluster;
		curr_cluster = fat16_get_next_cluster(fat_stream, curr_cluster, fat_table_offset);
		cluster_count++;
	};
	if (curr_cluster >= FAT16_VALUE_END_OF_CHAIN) {
		return;
	};
	fat16_create_fat_entry(dev, prev_cluster, FAT16_VALUE_END_OF_CHAIN);

	while (curr_cluster >= 2 && curr_cluster < FAT16_VALUE_END_OF_CHAIN) {
		uint16_t next_cluster = fat16_get_next_cluster(fat_stream, fat_table_offset, curr_cluster);
		fat16_create_fat_entry(dev, curr_cluster, FAT16_VALUE_FREE);
		curr_cluster = next_cluster;
	};
	return;
};

uint32_t fat16_scan_dir(ata_t* dev, uint32_t dir_cluster, const uint8_t* filename)
{
	const uint32_t partition_offset = 0x100000;
	stream_t fat_stream = {}, data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

	uint16_t curr_cluster = dir_cluster;

	while (curr_cluster < FAT16_VALUE_END_OF_CHAIN) {
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		uint8_t buffer[max_cluster_size_bytes];

		if (stream_read(&data_stream, buffer, max_cluster_size_bytes) < 0) {
			printf("[FAT16] ERROR: Cluster %d cannot be read.\n", curr_cluster);
			return 0;
		};
		fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;
		const size_t num_entries = max_cluster_size_bytes / sizeof(fat16_dir_entry_t);

		for (size_t i = 0; i < num_entries; i++, entry++) {
			if (memcmp(entry->file_name, filename, 11) == 0) {
				printf("[FAT16] INFO: File '%s' found in Cluster %d.\n", filename, curr_cluster);
				return curr_cluster;
			};
		};
		curr_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	return 0;
};

uint32_t fat16_get_parent_cluster(ata_t* dev, const uint8_t* filename)
{
	const uint32_t partition_offset = 0x100000;
	stream_t fat_stream = {}, data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

	uint32_t root_dir_sector = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
	uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);

	stream_seek(&data_stream, root_dir_sector);
	uint8_t buffer[root_dir_size];

	if (stream_read(&data_stream, buffer, root_dir_size) < 0) {
		return 0;
	};
	fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;

	for (size_t i = 0; i < fat16_header.bpb.root_ent_cnt; i++, entry++) {
		if (entry->attributes & DIRECTORY) {
			const uint32_t folder_cluster = fat16_get_combined_cluster(entry->high_cluster, entry->low_cluster);

			if (folder_cluster >= 2) {
				const uint32_t parent_cluster = fat16_scan_dir(dev, folder_cluster, filename);

				if (parent_cluster != 0) {
					return folder_cluster;
				};
			};
		};
	};
	return 0;
};

uint32_t fat16_find_entry_offset(ata_t* dev, uint32_t folder_cluster, const uint8_t* filename)
{
	const uint32_t partition_offset = 0x100000;
	stream_t fat_stream = {}, data_stream = {};
	stream_init(&fat_stream, dev);
	stream_init(&data_stream, dev);
	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

	uint16_t curr_cluster = folder_cluster;
	while (curr_cluster < FAT16_VALUE_END_OF_CHAIN) {
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		uint8_t buffer[max_cluster_size_bytes];
		if (stream_read(&data_stream, buffer, max_cluster_size_bytes) < 0) {
			printf("[FAT16] ERROR: Cluster %d can't be read.\n", curr_cluster);
			return 0;
		};
		fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;
		size_t num_entries = max_cluster_size_bytes / sizeof(fat16_dir_entry_t);

		for (size_t i = 0; i < num_entries; i++, entry++) {
			if (memcmp(entry->file_name, filename, 11) == 0) {
				printf("[FAT16] INFO: File '%s' found in Cluster %d.\n", filename, curr_cluster);
				return data_pos + (i * sizeof(fat16_dir_entry_t));
			};
		};
		curr_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	return 0;
};

size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks)
{
	if (!dev || !internal || !buffer || n_bytes == 0 || n_blocks == 0) {
		return -EINVAL;
	};
	const uint32_t partition_offset = 0x100000;

	fat16_fd_t* fd = (fat16_fd_t*)internal;
	fat16_dir_entry_t* file_entry = fd->entry->file;
	const uint16_t start_cluster = fat16_get_combined_cluster(file_entry->high_cluster, file_entry->low_cluster);
	size_t bytes_written = 0;
	size_t remaining_bytes = n_bytes * n_blocks;

	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;

	stream_t data_stream = {};
	stream_init(&data_stream, dev);

	uint32_t cluster_offset = 0;
	uint16_t curr_cluster = start_cluster;

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	const uint32_t fat_table_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	stream_seek(&fat_stream, fat_table_offset);

	const size_t existing_clusters = fat16_get_cluster_chain_length(&fat_stream, max_cluster_size_bytes, curr_cluster, partition_offset);
	const size_t required_clusters = (n_bytes + max_cluster_size_bytes - 1) / max_cluster_size_bytes;

	const size_t additional_clusters = (required_clusters > existing_clusters) ? required_clusters - existing_clusters : 0;
	stream_seek(&fat_stream, fat_table_offset);

	while (remaining_bytes) {
		const size_t free_space_in_cluster = max_cluster_size_bytes - cluster_offset;
		const size_t write_size = (remaining_bytes > free_space_in_cluster) ? free_space_in_cluster : remaining_bytes;
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos + cluster_offset);
		const int32_t res = stream_write(&data_stream, buffer + bytes_written, write_size);

		if (res < 0) {
			printf("[ERROR] Writing Error.\n");
			return -EIO;
		};
		bytes_written += write_size;
		remaining_bytes -= write_size;
		cluster_offset += write_size;

		if (cluster_offset >= max_cluster_size_bytes && remaining_bytes > 0) {
			uint16_t next_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);

			if (next_cluster >= FAT16_VALUE_END_OF_CHAIN) {
				if (!additional_clusters) {
					return -ENOSPC;
				};
				const uint16_t new_cluster = fat16_get_next_free_cluster(&fat_stream, partition_offset);

				if (!new_cluster) {
					return -ENOSPC;
				};
				// Link the curr cluster to the new cluster
				fat16_create_fat_entry(dev, curr_cluster, new_cluster);
				// Mark the last new sector at EOF
				fat16_create_fat_entry(dev, new_cluster, FAT16_VALUE_END_OF_CHAIN);
				next_cluster = new_cluster;
			};
			curr_cluster = next_cluster;
			cluster_offset = 0;
		};
	};
	// Case if required_clusters < existing_clusters, we must free the clusters..
	if (required_clusters < existing_clusters) {
		fat16_free_unused_clusters(dev, &fat_stream, curr_cluster, required_clusters, existing_clusters, fat_table_offset);
	};
	if (bytes_written > file_entry->file_size) {
		file_entry->file_size = bytes_written;
		const uint32_t parent_cluster = fat16_get_parent_cluster(dev, fd->entry->file->file_name);
		const uint32_t entry_offset = fat16_find_entry_offset(dev, parent_cluster, fd->entry->file->file_name);

		if (!entry_offset) {
			return -EIO;
		};
		fat16_time_t current_time = fat16_get_curr_time();
		fat16_date_t current_date = fat16_get_curr_date();

		file_entry->modification_time = fat16_get_packed_time(current_time);
		file_entry->modification_date = fat16_get_packed_date(current_date);

		stream_t dir_stream = {};
		stream_init(&dir_stream, dev);
		stream_seek(&dir_stream, entry_offset);
		const int32_t res = stream_write(&dir_stream, (uint8_t*)file_entry, sizeof(fat16_dir_entry_t));

		if (res < 0) {
			printf("[ERROR] Writing Error\n");
			return -EIO;
		};
		printf("[FAT16] SUCCESS: File-Entry updated: Size %d Bytes, Timestamp %d:%d:%d %d-%d-%d\n", file_entry->file_size, current_time.hour,
		       current_time.minute, current_time.second, current_date.day, current_date.month, current_date.year);
	};
	return bytes_written;
};

int32_t fat16_readdir(ata_t* dev, void* internal, vfs_dirent_t* dir, uint32_t dir_offset)
{
	// Each fat16 cluster is 8192 bytes, we must iterate through all 32byte fat_dir_entry_t's to find all dirs and files
	// we must ensure that we use the dir_offset (0...1...2...n) to skip n entries through the buffer :D
	// for example if dir_offset is 2 we must jump over 2x32 byte fat_dir_entry_t's und write the 3 entrie into the vfs_dirent_t* dir buffer
	// step 1) define par offset
	const uint32_t partition_offset = 0x100000;
	// step 2) Cast the vfs internal back to the concrete fat16 descriptor
	fat16_fd_t* fat16_descriptor = internal;
	// We can only read through directories, files dont make sense xD
	if (fat16_descriptor->entry->type != FAT16_ENTRY_TYPE_DIRECTORY) {
		return -EINVAL;
	};
	const uint16_t start_sector = fat16_descriptor->entry->dir->start_sector;
	const uint16_t end_sector = fat16_descriptor->entry->dir->end_sector;
	const uint8_t* filename = fat16_descriptor->entry->dir->entry->file_name;

	uint32_t root_dir_start = fat16_get_root_dir_offset(&fat16_header.bpb);
	const uint32_t root_dir_size = fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t);
	const uint32_t root_dir_end = root_dir_start + root_dir_size;

	const bool is_root_dir =
	    (((start_sector * fat16_header.bpb.byts_per_sec) == root_dir_start) && ((end_sector * fat16_header.bpb.byts_per_sec) == root_dir_end));

	if (is_root_dir && filename[0] == '\0') {
		// It must be a call to '/' :D
		// we just give our root_dir_stream the correct start_sector and lets eat the 512 dir entries in 32 byte chunks :D
		// we must check that we use the dir_offset
		// return if we reached all 512 entries with the dir_offset

		if (dir_offset >= fat16_header.bpb.root_ent_cnt) {
			return 0;
		};
		stream_t root_dir_stream = {};
		stream_init(&root_dir_stream, dev);
		uint32_t offset_within_root = dir_offset * sizeof(fat16_dir_entry_t);
		const uint32_t root_dir_abs = fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset);
		stream_seek(&root_dir_stream, root_dir_abs + offset_within_root);

		fat16_dir_entry_t entry = {};
		const int32_t res = stream_read(&root_dir_stream, (uint8_t*)&entry, sizeof(fat16_dir_entry_t));

		if (res < 0) {
			printf("[FAT16] ERROR: Failed to read Root Directory.\n");
			return 0x0;
		};

		while (entry.file_name[0] == FREE || entry.file_name[0] == DELETED) {
			while (entry.attributes == LFN) {
				dir_offset++;

				if (dir_offset >= fat16_header.bpb.root_ent_cnt) {
					return 0;
				};
			};
			dir_offset++;

			if (dir_offset >= fat16_header.bpb.root_ent_cnt) {
				return 0;
			};
			offset_within_root = dir_offset * sizeof(fat16_dir_entry_t);
			stream_seek(&root_dir_stream, root_dir_abs + offset_within_root);
			const int32_t res = stream_read(&root_dir_stream, (uint8_t*)&entry, sizeof(fat16_dir_entry_t));

			if (res < 0) {
				printf("[FAT16] ERROR: Failed to read Root Directory Entry.\n");
				return 0x0;
			};
		};
		fat16_get_userland_filename_from_native(dir->name, entry.file_name);
		dir->type = (entry.attributes & DIRECTORY) ? 1 : 0;
		dir->size = (dir->type == 1) ? 0 : (entry.file_size < 0) || (-entry.file_size) ? 0 : entry.file_size;
		dir->modified_time = entry.modification_time;
		return 1;
	};
	// step 3) no root dir? we must use the fat chain. we just use cluster from fat16_descriptor->entry->dir->entry->low_cluster to fed into the
	// stream :D build up a fat stream and put it into the right fat absolute offset :D i think we have a func for this then build a uint8_t
	// value[2] and give it into a func which use this buff to fed into the fat table 16 byte value in this value is the next cluster contained,
	// which we must use for a another function read_next_cluster, i think we have a func for this <: the next logic is simliar to the logic above
	// we use the dir_offset to read the whole 8192 byte cluster until we found end of cluster chain xD
	const uint16_t start_cluster =
	    fat16_get_combined_cluster(fat16_descriptor->entry->dir->entry->high_cluster, fat16_descriptor->entry->dir->entry->low_cluster);

	const uint16_t max_cluster_size_bytes = fat16_header.bpb.sec_per_clus * fat16_header.bpb.byts_per_sec;
	const uint32_t num_entries_per_cluster = max_cluster_size_bytes / sizeof(fat16_dir_entry_t);

	uint8_t buffer[max_cluster_size_bytes];

	stream_t data_stream = {};
	stream_init(&data_stream, dev);

	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	const uint32_t fat_table_offset = fat16_get_fat_table_absolute(&fat16_header.bpb, partition_offset);
	stream_seek(&fat_stream, fat_table_offset);

	uint16_t curr_cluster = start_cluster;

	while (curr_cluster != FAT16_VALUE_END_OF_CHAIN) {
		const uint32_t sector = fat16_get_sector_from_cluster(curr_cluster);
		const uint32_t data_pos = partition_offset + (sector * fat16_header.bpb.byts_per_sec);
		stream_seek(&data_stream, data_pos);

		const int32_t res = stream_read(&data_stream, buffer, max_cluster_size_bytes);

		if (res < 0) {
			printf("[FAT16] ERROR: Failed to read Cluster %d.\n", curr_cluster);
			return 0x0;
		};
		fat16_dir_entry_t* entry = (fat16_dir_entry_t*)buffer;

		for (size_t i = 0; i < num_entries_per_cluster; i++, entry++) {
			if (i == dir_offset) {
				while (entry->file_name[0] == FREE || entry->file_name[0] == DELETED || entry->attributes == LFN) {
					dir_offset++;

					if (dir_offset >= num_entries_per_cluster) {
						return 0;
					};
					entry++;
				};
				fat16_get_userland_filename_from_native(dir->name, entry->file_name);
				dir->type = (entry->attributes & DIRECTORY) ? 1 : 0;
				dir->size = (dir->type == 1) ? 0 : entry->file_size < 0 ? 0 : entry->file_size;
				dir->modified_time = entry->modification_time;
				return 1;
			};
		};
		curr_cluster = fat16_get_next_cluster(&fat_stream, partition_offset, curr_cluster);
	};
	return 0;
};