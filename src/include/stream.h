/**
 * @file stream.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>
#include <stddef.h>

#include "ata.h"

typedef struct Stream
{
    size_t pos;
    ATADev *dev;
} Stream;

void stream_init(Stream *self, ATADev *dev);
void stream_seek(Stream *self, const size_t pos);
int32_t stream_read(Stream *self, uint8_t *buffer, size_t n_bytes);

#endif