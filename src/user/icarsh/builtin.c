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
static void _help_builtin(const char* args);
static void _echo_builtin(const char* args);
static void _ls_builtin(const char* path);
static void _history_builtin(const char* args);
static void _cat_builtin(const char* path);
static void _heapstat_builtin(const char* args);
static void _unknown_builtin(const char* args);

void execute_builtin(const char* input);

typedef struct builtin {
	const char* name;
	builtin_handler_t handler;
} builtin_t;

const static builtin_t builtins[] = {
    {"exit", _exit_builtin},	   {"help", _help_builtin}, {"echo", _echo_builtin},	     {"ls", _ls_builtin},
    {"history", _history_builtin}, {"cat", _cat_builtin},   {"heapstat", _heapstat_builtin}, {0x0, 0x0},
};

#define BUILTIN_COUNT (sizeof(builtins) / sizeof(builtin_t))
#define ICARSH_INPUT_LIMIT 4096

static void _heapstat_builtin(const char* args)
{
	heap_dump();
	return;
};

static void _exit_builtin(const char* args)
{
	int status = 0;

	if (args && strlen(args) > 0) {
		status = atoi(args);
	};
	exit(status);
	printf("[!] TODO Cleanup Task Shell and Reboot ICARSH.BIN :)\n");
	return;
};

static void _history_builtin(const char* args)
{
	history_dump(&icarsh_history);
	return;
};

static void _echo_builtin(const char* args)
{
	const size_t len = strlen(args);

	if (!args || len == 0) {
		errno = EINVAL;
		write(STDOUT_FILENO, "\n", 1);
		return;
	};
	char* buf = malloc(len);

	if (!buf) {
		errno = ENOMEM;
		return;
	};
	memset(buf, 0, len);
	strncpy(buf, args, len);
	buf[len] = '\0';

	char* token = strtok(buf, " ");

	while (token) {
		printf("%s", token);
		token = strtok(0x0, " ");

		if (token) {
			printf(" ");
		};
	};
	printf("\n");
	free(buf);
	return;
};

static void _help_builtin(const char* args)
{
	printf("----[ Help ]----\n");
	printf("  `exit`          – TODO\n");
	printf("  `ls`            – LISTS FILES. YOU GOTTA PASS THE FULL ROOT PATH LIKE\n");
	printf("  `echo`          – SAY SOMETHING\n");
	printf("  `cat`           – PRINTS CONTENTS OF A FILE\n");
	printf("  `help`          – SHOWS THIS LIST AGAIN\n");
	printf("  `history`       – DUMP YOUR LAST COMMANDS\n");
	printf("  `heapstat`      – DUMP DYNAMIC MEMORY USAGE\n");
	return;
};

static void _cat_builtin(const char* path)
{
	FILE* f = fopen(path, "r");

	if (!f) {
		errno = EINVAL;
		printf("%s\n", strerror(errno));
		return;
	};
	char* buf = malloc(8192);

	if (!buf) {
		errno = ENOMEM;
		printf("[!] WHOA — OOM\n");
		return;
	};
	size_t n_read = 0;

	while ((n_read = fread(buf, 1, 8192, f)) > 0) {
		write(STDOUT_FILENO, buf, n_read);
	};
	free(buf);
	fclose(f);
	printf("\n");
	return;
};

static void _ls_builtin(const char* path)
{
	DIR* dir = opendir(path);

	if (!dir) {
		printf("%s\n", strerror(errno));
		return;
	};
	struct dirent* entry = {};

	while ((entry = readdir(dir)) != 0) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		};
		switch (entry->d_type) {
		case DT_DIR: {
			printf("/%s\n", entry->d_name);
			break;
		};
		case DT_REG: {
			printf("%s\n", entry->d_name);
			break;
		};
		default: {
			printf("%s\n", entry->d_name);
			break;
		};
		};
	};
	const int res = closedir(dir);

	if (res == -1) {
		printf("%s\n", strerror(errno));
	};
	return;
};

static void _unknown_builtin(const char* args)
{
	if (!args || !*args) {
		args = "";
	};
	printf("[!] WHOA — UNKNOWN\n", args);
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
	const size_t len = strlen(input);

	if (len >= ICARSH_INPUT_LIMIT) {
		printf("[!] WHOA — INPUT TOO LONG. KEEP IT SHORT & SHARP\n");
		return;
	};
	char* buf = malloc(len + 1);

	if (!buf) {
		printf("[!] WHOA — OOM\n");
		return;
	};
	strncpy(buf, input, len);
	buf[len] = '\0';

	const char* cmd = strtok(buf, " ");
	const char* args = strtok(NULL, "");

	if (!args) {
		args = "";
	};
	if (!cmd) {
		_unknown_builtin("");
		free(buf);
		return;
	};
	const builtin_handler_t builtin = _find_builtin(cmd);
	builtin(args);
	free(buf);
	return;
};