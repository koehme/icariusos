#include "builtin.h"
#include "history.h"
#include "readline/readline.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"

#define PROMPT "[icarSH]:~# "

int main(int argc, char* argv[])
{
	while (1) {
		execute_builtin(readline(PROMPT));
	};
	return 0;
};