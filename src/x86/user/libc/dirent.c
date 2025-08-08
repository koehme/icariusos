#include "dirent.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"

extern int getdents(int fd, struct dirent* buf, unsigned int count);

DIR* opendir(const char* path)
{
	const int fd = open(path, 0);

	if (fd < 0) {
		errno = ENOENT;
		return 0;
	};
	DIR* dirp = malloc(sizeof(DIR));

	if (!dirp) {
		errno = ENOMEM;
		close(fd);
		return 0;
	};
	dirp->fd = fd;
	dirp->has_entry = 0;
	return dirp;
};

struct dirent* readdir(DIR* dirp)
{
	if (!dirp || dirp->fd < 0) {
		errno = EBADF;
		return 0x0;
	};
	const int ret = getdents(dirp->fd, &dirp->current, sizeof(struct dirent));

	if (ret <= 0) {
		errno = EIO;
		dirp->has_entry = 0;
		return 0x0;
	};
	dirp->has_entry = 1;
	return &dirp->current;
};

int closedir(DIR* dirp)
{
	if (!dirp) {
		errno = EINVAL;
		return -1;
	};

	if (dirp->fd < 0) {
		errno = EBADF;
		return -1;
	};

	if (close(dirp->fd) < 0) {
		return -1;
	};
	dirp->fd = -1;
	dirp->has_entry = 0;
	free(dirp);
	return 0;
};