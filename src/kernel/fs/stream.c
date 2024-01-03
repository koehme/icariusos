#include "stream.h"

void stream_init(Stream *self, ATADisk *disk)
{
    self->pos = 0;
    self->disk = disk;
    return;
};

void stream_seek(Stream *self, const size_t pos)
{
    self->pos = pos;
    return;
};

int stream_read(Stream *self, uint8_t *buffer, size_t n_bytes)
{
    if (buffer == 0x0)
    {
        return -1;
    };
    // Calculate the LBA block needed for the read operation. The target block is saved into self->pos.
    // We calculate the right offset based on self->pos. For example, if self->pos is 0x200 (512),
    // the formula is 512 / block_size = 512 / 512 = 1.
    // Thus, we know we must start to read at block 1, and now we need the offset from this block.
    // For instance, if a user asks for pos 0x206 (518), we calculate the offset by
    // 0x206 % 512 = offset 6.
    const size_t block_size = self->disk->sector_size;
    const size_t lba_block = self->pos / block_size;
    const size_t offset = self->pos % block_size;
    // Create a temporary storage with a size of 512 bytes as uint_8.
    uint8_t sector_buffer[block_size];

    // Read the first sector/block of the calculated LBA block variable above using ata_read.
    // The data will be stored in sector_buffer.

    // Calculate the remaining_bytes. If n_bytes is greater than 512, set remaining_bytes to 512; otherwise, set it to n_bytes.

    // Copy all 512 bytes from the temporary buffer (sector_buffer) into the larger buffer provided by the parameter "buffer" in the function declaration.

    // Adjust the self->pos in the disk stream to the actual remaining_bytes.

    // Check if n_bytes is still greater than 512. If true, recursively call stream_read with n_bytes - 512.

    // Return 0 to indicate successful completion.
    return 0;
};