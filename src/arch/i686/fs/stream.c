/**
 * @file stream.c
 * @author Kevin
 * @copyright MIT
 * @brief Stream implementation for block device interaction.
 * @date 2024-11-16
 *
 * This file implements the `stream` interface, providing a basic mechanism to
 * interact with block devices, such as reading and seeking through data stored
 * on ATA devices. The stream abstraction simplifies reading data in a sequential
 * or random-access manner from block-based storage devices.
 *
 * The `stream` interface supports operations like seeking to a specific byte
 * position and reading a specified number of bytes from the device, leveraging
 * the underlying ATA driver for actual data transfer. This design allows for
 * efficient and modular data handling, making it an ideal fit for file systems
 * or other higher-level abstractions interacting with storage devices.
 */

#include "stream.h"
#include "string.h"

/* PUBLIC API */
void stream_init(stream_t* self, ata_t* dev);
void stream_seek(stream_t* self, const size_t pos);
int32_t stream_read(stream_t* self, uint8_t* buffer, const size_t n_bytes);

void stream_init(stream_t* self, ata_t* dev)
{
	self->pos = 0;
	self->dev = dev;
	return;
};

void stream_seek(stream_t* self, const size_t pos)
{
	self->pos = pos;
	return;
};

int32_t stream_read(stream_t* self, uint8_t* buffer, const size_t n_bytes)
{
	int32_t ata_status = -1;

	if (buffer == 0x0) {
		return -1;
	};
	const size_t block_size = self->dev->sector_size;
	size_t remaining_bytes = n_bytes;
	size_t read_size = n_bytes > block_size ? block_size : n_bytes;

	while (remaining_bytes) {
		const size_t lba_block = self->pos / block_size;
		const size_t offset = self->pos % block_size;
		ata_status = ata_read(self->dev, lba_block, 1);

		if (ata_status < 0) {
			return ata_status;
		};
		read_size = remaining_bytes > block_size ? block_size : remaining_bytes;

		for (size_t i = 0; i < read_size; i++) {
			*buffer++ = self->dev->buffer[offset + i];
		};
		self->pos += read_size;
		remaining_bytes -= read_size;
	};
	return ata_status;
};