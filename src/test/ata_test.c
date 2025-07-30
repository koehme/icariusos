/**
 * @file ata_test.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata_test.h"

void test_ata_write(ata_t* dev)
{
	if (!dev) {
		return;
	};
	const uint32_t test_sector = 4096;
	const char* test_data = "Hello IcariusOS!";

	uint8_t write_buffer[512] = {};
	memset(write_buffer, 0, 512);
	memcpy(write_buffer, test_data, strlen(test_data));

	for (int i = 0; i < 512; i++) {
		const uint8_t byte = write_buffer[i];
		const uint8_t high = (byte >> 4) & 0x0F;
		const uint8_t low = byte & 0x0F;
		printf("0x%x%x ", high, low);
	};
	printf("\n");
	printf("[KERNEL] Writing Test Data to Sector %d...\n", test_sector);
	int32_t res = ata_write(dev, test_sector, 1, write_buffer);

	outb(ATA_COMMAND_PORT, 0xE7);
	while (inb(ATA_COMMAND_PORT) & ATA_STATUS_BSY)
		;

	if (res < 0) {
		return;
	};
	printf("[KERNEL] Reading Back from Sector %d...\n", test_sector);
	res = ata_read(dev, test_sector, 1);

	if (res < 0) {
		return;
	};
	uint8_t read_buffer[512] = {};
	memcpy(read_buffer, dev->buffer, 512);

	for (int i = 0; i < 512; i++) {
		const uint8_t byte = read_buffer[i];
		const uint8_t high = (byte >> 4) & 0x0F;
		const uint8_t low = byte & 0x0F;
		printf("0x%x%x ", high, low);
	};
	printf("\n");

	if (memcmp(read_buffer, test_data, strlen(test_data)) == 0) {
		printf("[KERNEL] SUCCESS: ata_write()!\n");
	} else {
		printf("[KERNEL] ERROR: ata_write()!\n");
	};
	busy_wait(500000);
	return;
};