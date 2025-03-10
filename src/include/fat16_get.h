/**
 * @file fat16_get.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_GET
#define FAT16_GET

#include "icarius.h"
#include "stdio.h"
#include "stream.h"
#include "string.h"

#include "fat16.h"

uint16_t fat16_get_next_cluster(stream_t* fat_stream, const uint32_t partition_offset, const uint16_t curr_cluster);
fat16_dir_entry_t* fat16fat16_get_node_from_path_in_cluster_chain(ata_t* dev, const uint16_t start_cluster, const uint8_t* fat16_filename,
								  fat16_folder_t* fat16_folder, fat16_dir_entry_t* fat16_dir_entry);
uint32_t fat16_get_cluster_from_path(ata_t* dev, pathnode_t* path);
uint32_t fat16_get_free_root_dir_entry(ata_t* dev);
uint32_t fat16_get_free_dir_entry(ata_t* dev, uint32_t parent_cluster);
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
fat16_node_t* fat16_get_node_from_path(ata_t* dev, pathnode_t* path);
uint32_t fat16_get_cluster_from_descriptor(fat16_fd_t* fat16_descriptor);
uint32_t fat16_get_sector_from_cluster(const uint32_t cluster);
uint16_t fat16_get_cluster_chain_length(stream_t* fat_stream, const uint16_t max_cluster_size_bytes, const uint32_t start_cluster,
					const uint32_t partition_offset);
#endif