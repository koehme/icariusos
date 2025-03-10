/**
 * @file fat16_get.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16_get.h"
#include "fat16_create.h"

uint16_t fat16_get_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster)
{
	uint8_t fat_entry[2] = {};
	const uint32_t fat_entry_offset = partition_offset + fat16_header.bpb.rsvd_sec * fat16_header.bpb.byts_per_sec + curr_cluster * 2;
	stream_seek(fat_stream, fat_entry_offset);
	stream_read(fat_stream, fat_entry, 2 * sizeof(uint8_t));
	return ((uint16_t)fat_entry[0]) | ((uint16_t)fat_entry[1] << 8);
};

fat16_dir_entry_t* fat16fat16_get_node_from_path_in_cluster_chain(ata_t* dev, const uint16_t start_cluster, const uint8_t* fat16_filename,
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

		if (!fat16fat16_get_node_from_path_in_cluster_chain(dev, curr_cluster, (uint8_t*)curr->identifier, &parent_dir, &dir_entry)) {
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

uint32_t fat16_get_free_dir_entry(ata_t* dev, uint32_t parent_cluster) { return 0; };

uint16_t fat16_get_next_free_cluster(stream_t* fat_stream, const uint32_t partition_offset)
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
	const uint32_t last_root_dir_sector = first_root_dir_sector + root_dir_sectors - 1;
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
			fat16_get_native_filename_from_userland(native_dir_name, (uint8_t*)path);

			if (memcmp(curr_root_entry->file_name, native_dir_name, 11) == 0) {
				memcpy(entry, curr_root_entry, sizeof(fat16_dir_entry_t));
				folder->start_sector = first_root_dir_sector;
				folder->end_sector = last_root_dir_sector;
				folder->total = fat16_header.bpb.root_ent_cnt;
				break;
			};
		};
	};
	return entry;
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

	if (!path->next) {
		if (entry.file_name[0] && (entry.attributes & DIRECTORY)) {
			return fat16_create_node_from_dir(&folder, &entry);
		};

		if (entry.file_name[0] && (entry.attributes & ARCHIVE)) {
			return fat16_create_node_from_entry(&entry);
		};
	};
	const uint32_t root_dir_entry_cluster = fat16_get_combined_cluster(entry.high_cluster, entry.low_cluster);
	const uint32_t start_cluster = root_dir_entry_cluster;

	fat16fat16_get_node_from_path_in_cluster_chain(dev, start_cluster, native, &folder, &entry);

	if (entry.file_name[0] && (entry.attributes & DIRECTORY)) {
		return fat16_create_node_from_dir(&folder, &entry);
	};

	if (entry.file_name[0] && (entry.attributes & ARCHIVE)) {
		return fat16_create_node_from_entry(&entry);
	};
	return 0x0;
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

uint32_t fat16_get_sector_from_cluster(const uint32_t cluster)
{
	const uint32_t root_dir_sectors =
	    (fat16_header.bpb.root_ent_cnt * sizeof(fat16_dir_entry_t) + fat16_header.bpb.byts_per_sec - 1) / fat16_header.bpb.byts_per_sec;
	const uint32_t data_start_sector = fat16_header.bpb.rsvd_sec + (fat16_header.bpb.num_fats * fat16_header.bpb.fatsz16) + root_dir_sectors;
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