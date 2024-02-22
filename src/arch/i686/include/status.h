/**
 * @file status.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STATUS_H
#define STATUS_H

// Essential error codes in the os
typedef enum StatusCodes
{
    ENOENT = 0x02, // No such file or directory
    EIO = 0x05,    // I/O error
    EBADF = 0x09,  // Bad file number
    ENOMEM = 0x0C, // Not enough core
    EINVAL = 0x16, // Invalid argument
} StatusCodes;

#endif