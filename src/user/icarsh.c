#include "builtin.h"
#include "readline/readline.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define PROMPT "[icarSH]:~# "

int main(int argc, char* argv[])
{
	while (1) {
		const char* input = readline(PROMPT);

		if (!input) {
			continue;
		};
		execute_builtin(input);
		write(1, "\n", 1);
	};
	return 0;
};