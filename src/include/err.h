/**
 * @file err.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ERR_H
#define ERR_H

// Essential error codes in the os
typedef enum ErrorCode
{
    EIO = 5,     // I/O error
    ENOMEM = 12, // Insufficient memory
    EACCES = 13, // Permission denied
    ENOENT = 2,  // No such file or directory
    EINVAL = 22, // Invalid argument
    EEXIST = 17, // File already exists
    EPIPE = 32,  // Broken pipe
    EDEADLK = 45 // Deadlock condition
} ErrorCode;

#endif