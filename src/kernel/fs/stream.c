#include "stream.h"

void stream_init(Stream *self, const ATADisk *disk)
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

size_t stream_read(Stream *self, uint8_t *buffer, const size_t n_bytes)
{
    // Calculate the needed lba block. the target block is saved into self->pos
    // We can calculate the right offset through self->pos is for example 0x200 = 512
    // so formula is 512 / block_size = 512 / 512 = 1
    // so we know we must start to read at block 1 now we need the offset from this block
    // consider fow example a user ask for pos 0x206 which will be 518 so we must calculate the offset by
    // 0x206 % 512 = offset 6
    const size_t block_size = self->disk->sector_size;
    const size_t lba_block = self->pos / block_size;
    const size_t offset = self->pos % block_size;
    return 0;
};