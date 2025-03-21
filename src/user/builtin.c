#include "builtin.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

typedef struct builtin {
	const char* name;
	void (*handler)(void);
} builtin_t;

void exit_builtin(void)
{
	exit(0);
	return;
};

void help_builtin(void)
{
	printf("\nAvailable Built-in Commands:\n");
	printf("  `exit` - Quit the icariusOS Shell\n");
	printf("  `ls` - Peek into the filesystem\n");
	printf("  `help` - Show all built-ins\n");
	return;
};

void ls_builtin(void)
{
	char buf[256] = {"A:/TMP/LOG.TXT"};
	int fd = open(buf, 0);

	printf("\n");

	if (fd < 0) {
		printf("[ERROR] Failed to open directory %s!\n", buf);
	} else {
		printf("[SUCCESS] Opened directory %s! FD: %d\n", buf, fd);

		if (close(fd) == 0) {
			printf("[SUCCESS] Directory %s closed.\n", buf);
		} else {
			printf("[WARNING] Directory %s not closed.\n", buf);
		};
	};
	return;
};

void unknown_builtin(const char* input)
{
	printf("\nUnknown Command: %s\n", input);
	printf("Try 'help' to see available Commands.");
	return;
};

builtin_t builtins[] = {
    {"exit", exit_builtin},
    {"help", help_builtin},
    {"ls", ls_builtin},
};

void execute_builtin(const char* input)
{
	for (int i = 0; builtins[i].name != 0x0; i++) {
		if (strcmp(input, builtins[i].name) == 0) {
			builtins[i].handler();
			return;
		};
	};
	unknown_builtin(input);
	return;
};