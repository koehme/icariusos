#include "readline/readline.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define BUFFER_SIZE 1024

char* readline(const char* prompt)
{
	if (prompt) {
		write(1, prompt, strlen(prompt));
	};
	size_t pos = 0;
	static char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	char c;

	while (1) {
		if (read(0, &c, 1) <= 0) {
			return 0x0;
		};

		if (c == 0xD || pos >= BUFFER_SIZE - 1) {
			buffer[pos] = '\0';
			return buffer;
		};
		write(1, &c, 1);
		buffer[pos++] = c;
	};
	return 0x0;
};