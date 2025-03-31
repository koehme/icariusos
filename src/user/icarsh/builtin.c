#include "builtin.h"
#include "dirent.h"
#include "errno.h"
#include "history.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"
#include "unistd.h"

typedef void (*builtin_handler_t)(const char* args);
static void _exit_builtin(const char* args);
static void _history_builtin(const char* args);
static void _echo_builtin(const char* args);
static void _help_builtin(const char* args);
static void _ls_builtin(const char* path);
static void _unknown_builtin(const char* args);
void execute_builtin(const char* input);
static void _cat_builtin(const char* path);

#define BUILTIN_COUNT (sizeof(builtins) / sizeof(builtin_t))
#define MAX_INPUT_LEN 256

typedef struct builtin {
	const char* name;
	builtin_handler_t handler;
} builtin_t;

builtin_t builtins[] = {
    {"exit", _exit_builtin},
    {"help", _help_builtin},
    {"echo", _echo_builtin},
    {"ls", _ls_builtin},
    {"history", _history_builtin},
    {"cat", _cat_builtin},
    {0x0, 0x0},
};

static void _exit_builtin(const char* args)
{
	int status = 0;

	if (args && strlen(args) > 0) {
		status = atoi(args);
	};
	exit(status);
	return;
};

static void _history_builtin(const char* args)
{
	history_dump(&icarsh_history);
	return;
};

static void _echo_builtin(const char* args)
{
	if (!args || strlen(args) == 0) {
		write(STDOUT_FILENO, "\n", 1);
		return;
	};
	char buf[256];
	strncpy(buf, args, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	char* token = strtok(buf, " ");

	while (token) {
		printf("%s", token);
		token = strtok(0x0, " ");

		if (token) {
			printf(" ");
		};
	};
	printf("\n");
	return;
};

static void _help_builtin(const char* args)
{
	printf("----[ Help ]----\n");
	printf("  `exit`          – CLOSES ICARSH AND DROPS YOU BACK TO ICARIUSOS RING 0\n");
	printf("  `ls`            – LISTS FILES. YOU GOTTA PASS THE FULL ROOT PATH LIKE\n");
	printf("  `echo`          – SAY SOMETHING\n");
	printf("  `cat`           – PRINTS CONTENTS OF A FILE\n");
	printf("  `help`          – SHOWS THIS LIST AGAIN\n");
	printf("  `history`       – DUMP YOUR LAST COMMANDS\n");
	return;
};

static void _cat_builtin(const char* path)
{
	FILE* f = fopen(path, "r");

	if (!f) {
		printf("Cannot open '%s'\n", path);
		return;
	}
	char buf[8192];

	while (!feof(f)) {
		const size_t nread = fread(buf, 1, sizeof(buf), f);

		if (nread > 0) {
			write(STDOUT_FILENO, buf, nread);
		};
	};
	fclose(f);
	printf("\n");
	return;
};

static void _ls_builtin(const char* path)
{
	DIR* dir = opendir(path);

	if (!dir) {
		const char* shown_path = (path && strlen(path) > 0) ? path : "?";
		printf("Opening %s %s\n", shown_path, strerror(errno));
		return;
	};
	struct dirent* entry = {};

	while ((entry = readdir(dir)) != 0) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		};
		if (entry->d_type == DT_DIR) {
			printf("/%s\n", entry->d_name);
		} else if (entry->d_type == DT_REG) {
			printf("%s\n", entry->d_name);
		} else {
			printf("%s\n", entry->d_name);
		};
	};
	const int res = closedir(dir);

	if (res == -1) {
		printf("Couldn't close that dir – %s\n", strerror(errno));
	};
	return;
};

static void _unknown_builtin(const char* args)
{
	if (!args || !*args) {
		args = "";
	};
	printf("Unknown '%s'\n", args);
	return;
};

static builtin_handler_t _find_builtin(const char* cmd)
{
	for (size_t i = 0; builtins[i].name; i++) {
		if (strcmp(cmd, builtins[i].name) == 0)
			return builtins[i].handler;
	};
	return _unknown_builtin;
};

void execute_builtin(const char* input)
{
	char buf[MAX_INPUT_LEN];
	const size_t len = strlen(input);

	if (len >= sizeof(buf)) {
		printf("Input exceeds max length (%d chars). Try something shorter.\n", sizeof(buf) - 1);
		return;
	};
	strncpy(buf, input, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	const char* cmd = strtok(buf, " ");
	const char* args = strtok(0x0, "");

	if (!args) {
		args = "";
	};

	if (!cmd) {
		_unknown_builtin("");
		return;
	};
	const builtin_handler_t handler = _find_builtin(cmd);
	handler(args);
	return;
};