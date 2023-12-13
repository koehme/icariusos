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
} ErrorCode;

#endif