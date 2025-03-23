#include "syscall.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_GETDENTS 141

static inline int syscall(int num, int arg1, int arg2, int arg3)
{
	int ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
	return ret;
};


int read(int fd, void* buf, int count)
{
	const int ret = syscall(SYS_READ, fd, (int)buf, count);
	return ret;
};

int write(int fd, const void* buf, int count)
{
	const int ret = syscall(SYS_WRITE, fd, (int)buf, count);
	return ret;
};

void exit(int status)
{
	const int ret = syscall(SYS_EXIT, status, 0, 0);
	(void)ret;
};

int close(int fd)
{
	const int ret = syscall(SYS_CLOSE, fd, 0, 0);
	return ret;
};

int open(const char* path, int flags)
{
	const int ret = syscall(SYS_OPEN, (int)path, flags, 0);
	return ret;
};

int getdents(int fd, struct dirent* buf, unsigned int count)
{
	const int ret = syscall(SYS_GETDENTS, fd, (int)buf, count);
	return ret;
};