/**
 * @file fat16.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FAT16_H
#define FAT16_H

#define FAT16_DEBUG_DELAY 0

#include <stdint.h>

#include "ata.h"
#include "vfs.h"

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
		fat16_dir_entry_t* file;
		fat16_folder_t* dir;
	};
	fat16_entry_t type;
} fat16_node_t;

typedef struct fat16_fd {
	fat16_node_t* entry;
	uint32_t pos;
} fat16_fd_t;

extern fat16_internal_header_t fat16_header;
extern fs_t fat16;

fs_t* fat16_init(void);
int32_t fat16_resolve(ata_t* dev);
void* fat16_open(ata_t* dev, pathnode_t* path, const uint8_t mode);
size_t fat16_read(ata_t* dev, void* internal, uint8_t* buffer, const size_t n_bytes, const size_t n_blocks);
int32_t fat16_close(void* internal);
int32_t fat16_stat(ata_t* dev, void* internal, vstat_t* vstat_t);
int32_t fat16_seek(void* internal, const uint32_t offset, const uint8_t origin);
size_t fat16_write(ata_t* dev, void* internal, const uint8_t* buffer, size_t n_bytes, size_t n_blocks);
void fat16_dump(ata_t* dev, uint16_t start_cluster, size_t depth);

#endif