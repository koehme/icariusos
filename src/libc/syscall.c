#include <syscall.h>

static inline int syscall(int num, int arg1, int arg2, int arg3)
{
	int ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
	return ret;
};

int write(int fd, const void* buf, int count) { return syscall(4, fd, (int)buf, count); };

int read(int fd, void* buf, int count) { return syscall(3, fd, (int)buf, count); };

void exit(int status) { syscall(1, status, 0, 0); };