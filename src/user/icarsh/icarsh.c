#include "builtin.h"
#include "history.h"
#include "readline/readline.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define PROMPT "[icarSH]:~# "

int main(int argc, char* argv[])
{
	while (1) {
		const char* line = readline(PROMPT);

		if (!line) {
			continue;
		};
		execute_builtin(line);
	};
	return 0;
};