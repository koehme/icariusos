#ifndef _SYSCALL_H
#define _SYSCALL_H

int write(int fd, const void* buf, int count);
int read(int fd, void* buf, int count);
void exit(int status);

#endif