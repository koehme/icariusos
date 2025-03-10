/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-13
 */

#include "fat16.h"
#include "icarius.h"
#include "stream.h"
#include "string.h"

#include "fat16_create.h"
#include "fat16_dump.h"
#include "fat16_get.h"

fs_t fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .read_cb = 0x0,
    .close_cb = 0x0,
    .stat_cb = 0x0,
    .seek_cb = 0x0,
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

static bool _is_valid_fat16_header(const fat16_internal_header_t* header)
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
	fat16_dump_base_header(&fat16_header.bpb, "", 0);
	fat16_dump_ebpb_header(&fat16_header.ebpb, "", 0);
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

	if (!entry && mode == WRITE) {
		printf("[FAT16] File not found - Creating FAT16 Entry\n");
		entry = fat16_create_file(dev, path);

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

size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks)
{
	if (!dev || !internal || !buffer || n_bytes == 0 || n_blocks == 0) {
		return -EINVAL;
	};
	fat16_fd_t* fd = (fat16_fd_t*)internal;
	fat16_dir_entry_t* file_entry = fd->entry->file;
	uint16_t curr_cluster = fat16_get_combined_cluster(file_entry->high_cluster, file_entry->low_cluster);
	size_t bytes_written = 0;
	size_t bytes_to_write = n_bytes * n_blocks;
	return 0;
};