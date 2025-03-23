#include <errno.h>

const char* strerror(int errnum)
{
	switch (errnum) {
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case ESRCH:
		return "No such process";
	case EINTR:
		return "Interrupted system call";
	case EIO:
		return "I/O error";
	case EBADF:
		return "Bad file descriptor";
	case EINVAL:
		return "Invalid argument";
	case ENOMEM:
		return "Out of memory";
	case EMFILE:
		return "Too many open files";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EAGAIN:
		return "Resource temporarily unavailable";
	default:
		return "Unknown error";
	};
	return 0x0;
};