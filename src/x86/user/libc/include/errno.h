#ifndef ERRNO_H
#define ERRNO_H

extern int errno;

#define EPERM 1	   // Operation not permitted
#define ENOENT 2   // No such file or directory
#define ESRCH 3	   // No such process
#define EINTR 4	   // Interrupted system call
#define EIO 5	   // I/O error
#define EBADF 9	   // Bad file descriptor
#define EINVAL 22  // Invalid argument
#define ENOMEM 12  // Out of memory
#define EMFILE 24  // Too many open files
#define ENOTDIR 20 // Not a directory
#define EISDIR 21  // Is a directory
#define EAGAIN 11  // Try again (resource temporarily unavailable)

#endif