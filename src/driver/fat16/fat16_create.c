/**
 * @file fat16_create.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16_create.h"
#include "fat16_get.h"

#include "stream.h"
#include "string.h"

// Creates a new file in the FAT16 file system
fat16_node_t* fat16_create_file(ata_t* dev, pathnode_t* path)
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
	const uint32_t parent_cluster =
	    is_root_dir ? fat16_get_root_dir_absolute(&fat16_header.bpb, partition_offset) : fat16_get_cluster_from_path(dev, parent);
	// Step 4: If the parent directory does not exist, abort file creation
	if (!parent_cluster) {
		printf("[FAT16] ERROR: Parent directory '%s' not found. File creation failed.\n", is_root_dir ? "/" : parent->identifier);
		return 0x0;
	};
	printf("[FAT16] INFO: Creating File in %s (Cluster: %d)\n", is_root_dir ? "Root Directory" : parent->identifier, parent_cluster);
	// Step 5: Find a free directory entry
	const uint32_t free_entry = is_root_dir ? fat16_get_free_root_dir_entry(dev) : fat16_get_free_dir_entry(dev, parent_cluster);

	if (!free_entry) {
		printf("[FAT16] ERROR: No free Space in Directory for '%s'\n", path->identifier);
		return 0x0;
	};
	// Step 6: Find a free cluster for the file's data storage
	stream_t fat_stream = {};
	stream_init(&fat_stream, dev);
	const uint32_t free_cluster = fat16_get_next_free_cluster(&fat_stream, partition_offset);

	if (!free_cluster) {
		printf("[FAT16] ERROR: No free clusters available. File creation failed.\n");
		return 0x0;
	};
	printf("[FAT16] INFO: Allocated Cluster %d for new File '%s'\n", free_cluster, path->identifier);
	// Step 7: Prepare the file entry
	fat16_dir_entry_t new_entry = {};
	memset(&new_entry, 0, sizeof(fat16_dir_entry_t));
	fat16_get_native_filename_from_userland(new_entry.file_name, (uint8_t*)path->identifier);
	new_entry.attributes = ARCHIVE;
	new_entry.high_cluster = (free_cluster >> 16) & 0xFFFF;
	new_entry.low_cluster = free_cluster & 0xFFFF;
	new_entry.file_size = 0;
	// fat16_dump_fat_table(dev);
	// Step 8: Mark the allocated cluster in the FAT as EOF (end of file)
	fat16_create_fat_entry(dev, free_cluster, FAT16_VALUE_END_OF_CHAIN);
	// Step 9: Write the file entry into the directory
	if (is_root_dir) {
		fat16_create_root_dir_entry(dev, free_entry, &new_entry);
	} else {
		fat16_create_dir_entry(dev, parent_cluster, &new_entry);
	};
	// Step 10: Return a FAT16 node representing the new file
	fat16_node_t* fat16_entry = kzalloc(sizeof(fat16_node_t));
	fat16_entry->file = kzalloc(sizeof(fat16_dir_entry_t));
	memcpy(fat16_entry->file, &new_entry, sizeof(fat16_dir_entry_t));
	fat16_entry->type = FAT16_ENTRY_TYPE_FILE;
	// Convert the native fat16 filename to more userland friendly 'BLA     .BIN' to 'BLA.BIN'
	char userland[11];
	fat16_get_userland_filename_from_native(userland, fat16_entry->file->file_name);
	printf("[FAT16] SUCCESS: Created file '%s' in '%s'\n", userland, is_root_dir ? "/" : parent->identifier);
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
	printf("[FAT16] SUCCESS: Updated FAT entry for Cluster %d to 0x%x\n", cluster, value);
	return true;
};

static bool fat16_create_root_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry)
{
	if (!dev || !new_entry) {
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

bool fat16_create_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry) { return false; };

fat16_node_t* fat16_create_node_from_entry(fat16_dir_entry_t* entry)
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
	node->dir->entry = kzalloc(sizeof(fat16_dir_entry_t));

	if (!node->dir->entry) {
		kfree(node->dir);
		kfree(node);
		return 0x0;
	};
	node->type = FAT16_ENTRY_TYPE_DIRECTORY;
	memcpy(node->dir->entry, entry, sizeof(fat16_dir_entry_t));
	memcpy(node->dir, folder, sizeof(fat16_folder_t));
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