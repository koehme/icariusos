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
    EIO = 5,     // I/O error
    ENOMEM = 12, // Insufficient memory
} StatusCodes;

#endif