#include "builtin.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"

typedef void (*builtin_handler_t)(const char* args);

typedef struct builtin {
	const char* name;
	builtin_handler_t handler;
} builtin_t;


static void _exit_builtin(const char* args)
{
	int status = 0;

	if (args && strlen(args) > 0) {
		status = atoi(args);
	};
	exit(status);
	return;
};

static void _echo_builtin(const char* args)
{
	if (!args || strlen(args) == 0) {
		write(1, "\n", 1);
		return;
	};
	char buf[256];
	strncpy(buf, args, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	char* token = strtok(buf, " ");
	printf("\n");

	while (token) {
		printf("%s", token);
		token = strtok(0x0, " ");

		if (token) {
			printf(" ");
		};
	};
	return;
};

void help_builtin(const char* args)
{
	printf("\nAvailable Built-in Commands:\n");
	printf("  `exit` - Quit the icariusOS Shell\n");
	printf("  `ls` - Peek into the filesystem\n");
	printf("  `echo` - Print text to stdout\n");
	printf("  `help` - Show all built-ins\n");
	return;
};

static void _ls_builtin(const char* args)
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

static void _unknown_builtin(const char* args)
{
	printf("\nUnknown Command: %s\n", args);
	printf("Try 'help' to see available Commands.");
	return;
};

builtin_t builtins[] = {
    {"exit", _exit_builtin},
    {"help", help_builtin},
    {"echo", _echo_builtin},
    {"ls", _ls_builtin},
};

void execute_builtin(const char* input)
{
	char buf[256];
	strncpy(buf, input, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	const char* cmd = strtok(buf, " ");
	const char* args = strtok(0x0, "");

	for (int i = 0; builtins[i].name != 0x0; i++) {
		if (strcmp(cmd, builtins[i].name) == 0) {
			builtins[i].handler(args);
			return;
		};
	};
	_unknown_builtin(input);
	return;
};