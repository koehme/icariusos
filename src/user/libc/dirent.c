#include "dirent.h"
#include "errno.h"
#include "string.h"
#include "syscall.h"

#define MAX_OPEN_DIRS 16

static DIR dir_slots[MAX_OPEN_DIRS] = {[0 ... MAX_OPEN_DIRS - 1] = {.fd = -1}};

extern int getdents(int fd, struct dirent* buf, unsigned int count);

DIR* opendir(const char* path)
{
	const int fd = open(path, 0);

	if (fd < 0) {
		errno = ENOENT;
		return 0;
	};
	for (int i = 0; i < MAX_OPEN_DIRS; i++) {
		DIR* dirp = &dir_slots[i];

		if (dirp->fd == -1) {
			dirp->fd = fd;
			dirp->has_entry = 0;
			return dirp;
		};
	};
	close(fd);
	errno = EMFILE;
	return 0;
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
	return 0;
};