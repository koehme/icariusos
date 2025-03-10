/**
 * @file fat16_dump.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_DUMP
#define FAT16_DUMP

#include "fat16.h"
#include "icarius.h"
#include "stdio.h"
#include "stream.h"
#include "string.h"

void fat16_dump_base_header(const bpb_t* bpb, const char* msg, const int32_t delay);
void fat16_dump_ebpb_header(const ebpb_t* ebpb, const char* msg, const int32_t delay);
void fat_16_dump_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
void fat16_dump_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay);
void fat16_dump_root_dir(const bpb_t* bpb, stream_t* stream_t);
void fat16_dump_fat_table(ata_t* dev);
void fat16_dump_root_dir_entry_at(ata_t* dev, const int32_t i);

#endif