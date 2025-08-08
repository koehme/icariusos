#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY 0x0   // Open for reading only
#define O_WRONLY 0x1   // Open for writing only
#define O_RDWR 0x2     // Open for reading and writing
#define O_CREAT 0x40   // Create file if it does not exist
#define O_TRUNC 0x200  // Truncate size to 0
#define O_APPEND 0x400 // Writes append to end of file

#endif