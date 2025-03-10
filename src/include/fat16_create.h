/**
 * @file fat16_create.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_CREATE
#define FAT16_CREATE

#include "fat16.h"

fat16_node_t* fat16_create_file(ata_t* dev, pathnode_t* path);
bool fat16_create_fat_entry(ata_t* dev, uint32_t cluster, uint16_t value);
static bool fat16_create_root_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry);
bool fat16_create_dir_entry(ata_t* dev, uint32_t entry_sector, fat16_dir_entry_t* new_entry);
fat16_node_t* fat16_create_node_from_entry(fat16_dir_entry_t* entry);
fat16_node_t* fat16_create_node_from_dir(fat16_folder_t* folder, fat16_dir_entry_t* entry);
void fat16_create_file_stat(fat16_fd_t* fat16_descriptor, ata_t* dev, vstat_t* vstat, uint32_t max_cluster_size_bytes, uint32_t used_blocks);

#endif