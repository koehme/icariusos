/**
 * @file status.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STATUS_H
#define STATUS_H

#define ENOENT 0x02 // No such file or directory
#define EIO 0x05    // I/O error
#define EBADF 0x09  // Bad file number
#define ENOMEM 0x0C // Not enough core
#define EINVAL 0x16 // Invalid argument
#define EINVARG 2   // Invalid argument (duplicate of ENOENT)

#endif