#ifndef SYSCALL_H
#define SYSCALL_H

#include "dirent.h"

int write(int fd, const void* buf, int count);
int read(int fd, void* buf, int count);
void exit(int status);
int open(const char* path, int flags);
int close(int fd);
int getdents(int fd, struct dirent* buf, unsigned int count);

#endif