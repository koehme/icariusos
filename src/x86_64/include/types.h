/**
 * @file types.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include <stdint.h>
#include <uchar.h>

typedef int8_t s8;   // signed 8-bit integer (tiny signed values, flags, small enums)
typedef int16_t s16; // signed 16-bit integer (I/O data, short ranges, compat structs)
typedef int32_t s32; // signed 32-bit integer (general signed math, IDs, counters)
typedef int64_t s64; // signed 64-bit integer (large ranges, time, file offsets)

typedef uint8_t u8;   // unsigned 8-bit (raw bytes, buffers, chars in UTF-8)
typedef uint16_t u16; // unsigned 16-bit (hardware regs, short ranges, ports)
typedef uint32_t u32; // unsigned 32-bit (general unsigned math, sizes, masks)
typedef uint64_t u64; // unsigned 64-bit (addresses, big sizes, timestamps)

typedef uintptr_t uptr; // unsigned integer large enough to hold a pointer (address math)
typedef intptr_t sptr;	// signed integer large enough to hold a pointer (pointer diffs)

typedef uptr usize; // size of objects, array lengths, memory allocation sizes
typedef sptr ssize; // signed size (e.g. return values that can be negative: read/write)

typedef usize size_t;  // C standard type for object sizes (alias of usize)
typedef ssize ssize_t; // POSIX type for signed sizes (alias of ssize)

typedef char8_t c8; // UTF-8 code unit (text bytes, not generic data bytes)
