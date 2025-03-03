#include "syscall.h"
#include <string.h>
#include <sys/types.h>

int puts(const char* s)
{
	if (!s) {
		return -1;
	};
	const size_t len = strlen(s);
	ssize_t written;

	if (len > 0) {
		written = write(1, s, len);

		if (written == -1 || (size_t)written != len) {
			return -1;
		};
	};
	written = write(1, "\n", 1);

	if (written == -1 || written != 1) {
		return -1;
	};
	return 0;
};