#include "fat16_dump.h"

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
	const fat16_time_t create_time = fat16_unpack_time(entry->create_time);
	const fat16_date_t create_date = fat16_unpack_date(entry->create_date);
	const fat16_date_t last_access_date = fat16_unpack_date(entry->last_access_date);
	const fat16_time_t mod_time = fat16_unpack_time(entry->modification_time);
	const fat16_date_t mod_date = fat16_unpack_date(entry->modification_date);

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
	return;
};

void fat16_dump_lfn_entry(size_t i, fat16_dir_entry_t* entry, const int32_t delay)
{
	if (entry->file_name[0] == 0x00) {
		return;
	};
	printf("==========================\n");
	printf("=   RootDirLFNEntry %d:\n", i);
	printf("==========================\n");
	printf("=   LFN: %s (Long File Name NOT SUPPORTED)\n", "-");
	printf("==========================\n");
	return;
};

void fat16_dump_root_dir(const bpb_t* bpb, stream_t* stream_t)
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