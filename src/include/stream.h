/**
 * @file stream.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>
#include "ata.h"

typedef struct Stream
{
    uint64_t pos;
    ATADisk *disk;
} Stream;

void stream_init(Stream *self, const ATADisk *disk);
void stream_seek(Stream *self, const size_t pos);
size_t stream_read(Stream *self, uint8_t *buffer, const size_t n_bytes);

#endif