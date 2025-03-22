/**
 * @file vfs_test.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vfs_test.h"

void test_read(const char file[])
{
	printf("\n");
	printf("############################\n");
	printf("##      VFS READ TEST     ##\n");
	printf("##------------------------##\n");
	const int32_t fd = vfs_fopen(file, "r");

	if (fd < 0) {
		printf("[ERROR] Failed to Open File %s\n", file);
		return;
	};
	char buffer[1024] = {};

	if (vfs_fseek(fd, 0x2300, SEEK_SET) < 0) {
		printf("[ERROR] Failed to Seek in File Descriptor: %d\n", fd);
		vfs_fclose(fd);
		return;
	};

	if (vfs_fread(buffer, 10, 1, fd) <= 0) {
		printf("[ERROR] Failed to Read from File Descriptor: %d\n", fd);
		vfs_fclose(fd);
		return;
	};

	if (vfs_fread(buffer, 10, 1, fd) <= 0) {
		printf("[ERROR] Failed to Read from File Descriptor: %d\n", fd);
		vfs_fclose(fd);
		return;
	};
	printf("##        READ DATA:      ##\n");
	printf("##       %s\n", buffer);
	printf("############################\n");

	if (vfs_fclose(fd) < 0) {
		printf("[ERROR] Failed to Read from File Descriptor: %d\n", fd);
	};
	return;
};

void test_readdir(const char* path)
{
	const int32_t fd = vfs_fopen(path, "r");
	vfs_dirent_t dir = {};

	if (fd < 0) {
		printf("Error: Could not open %s\n", path);
		return;
	};
	printf("\n%s\n", path);
	printf("========================\n");

	while (vfs_readdir(fd, &dir)) {
		const int32_t name_length = strlen(dir.name);

		if (dir.type == 1 && strcmp(dir.name, ".") != 0 && strcmp(dir.name, "..") != 0) {
			printf("/");
		};
		printf("%s", dir.name);

		for (size_t i = name_length; i < 12; i++) {
			printf(" ");
		};

		if (dir.type == 1) {
			printf("[DIR]\n");
		} else {
			printf(" %d Bytes\n", dir.size);
		};
	};
	printf("\n");
	vfs_fclose(fd);
	return;
};

void test_write(char msg[], const uint32_t bytes)
{
	const int32_t fd = vfs_fopen("A:/TMP/LOG.TXT", "w");

	if (fd < 0) {
		return;
	};
	char buf[bytes];
	strncpy(buf, msg, bytes);
	vfs_fwrite(buf, bytes, 1, fd);
	vfs_fclose(fd);
	printf("[FAT16] LOG.TXT was successfully written.\n");
	return;
};
