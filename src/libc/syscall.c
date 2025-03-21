#include <syscall.h>

static inline int syscall(int num, int arg1, int arg2, int arg3)
{
	int ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
	return ret;
};

void exit(int status) { syscall(1, status, 0, 0); };

int read(int fd, void* buf, int count) { return syscall(3, fd, (int)buf, count); };

int write(int fd, const void* buf, int count) { return syscall(4, fd, (int)buf, count); };

int open(const char* path, int flags) { return syscall(5, (int)path, flags, 0); };

int close(int fd) { return syscall(6, fd, 0, 0); }