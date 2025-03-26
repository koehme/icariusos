#ifndef UNISTD_H
#define UNISTD_H

#include <dirent.h>
#include <sys/types.h>

int write(int fd, const void* buf, int count);
int read(int fd, void* buf, int count);
void exit(int status);
int open(const char* path, int flags);
int close(int fd);
int getdents(int fd, struct dirent* buf, unsigned int count);

#endif