#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main(int argc, char* argv[])
{
	char buf[1] = {0};
	puts("[icarSH]:~#");

	while (1) {
		const int bytes_read = read(0, buf, 1);

		if (bytes_read <= 0) {
			puts("\n[ERROR] Failed to Read. Exiting...\n");
			exit(1);
		};
		write(1, buf, 1);

		if (buf[0] == 'q') {
			puts("\nExiting icarSh...\n");
			exit(0);
		};
		memcpy(buf, "\0", sizeof(buf));
	};
	return 0;
};