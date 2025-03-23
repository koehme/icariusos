#include "builtin.h"
#include "dirent.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "syscall.h"

typedef void (*builtin_handler_t)(const char* args);
static void _exit_builtin(const char* args);
static void _echo_builtin(const char* args);
static void _help_builtin(const char* args);
static void _ls_builtin(const char* path);
static void _unknown_builtin(const char* args);
void execute_builtin(const char* input);

#define BUILTIN_COUNT (sizeof(builtins) / sizeof(builtin_t))

typedef struct builtin {
	const char* name;
	builtin_handler_t handler;
} builtin_t;

builtin_t builtins[] = {
    {"exit", _exit_builtin}, {"help", _help_builtin}, {"echo", _echo_builtin}, {"ls", _ls_builtin}, {0x0, 0x0},
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

static void _help_builtin(const char* args)
{
	printf("\n Welcome to the icariusOS Built-ins!\n");
	printf("Here's what you can do:\n\n");
	printf(" `exit`  – Peace out! Closes the shell.\n");
	printf(" `ls`    – Take a sneaky peek into the file jungle.\n");
	printf(" `echo`  – Say something. Literally just say it.\n");
	printf(" `help`  – Shows this rad list again.\n");
	printf("\nPro tip: Commands are case-sensitive. Don't yell unless you mean it.\n");
	return;
};

static void _ls_builtin(const char* path)
{
	DIR* dir = opendir(path);

	if (!dir) {
		printf("ls: tried opening '%s'... but the universe said: %s\n", path, strerror(errno));
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
	closedir(dir);
	return;
};

static void _unknown_builtin(const char* args)
{
	if (!args || !*args) {
		args = "";
	};
	printf("\nUnknown command: '%s'\n", args);
	printf("Not sure what that means, but I'm just a shell.\n");
	printf("Try 'help' to see what icariusOS do understand.\n");
};

static builtin_handler_t _find_builtin(const char* cmd)
{
	for (int i = 0; builtins[i].name; i++) {
		if (strcmp(cmd, builtins[i].name) == 0)
			return builtins[i].handler;
	};
	return _unknown_builtin;
};

void execute_builtin(const char* input)
{
	char buf[256];
	strncpy(buf, input, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	const char* cmd = strtok(buf, " ");
	const char* args = strtok(0x0, "");

	if (!cmd) {
		_unknown_builtin("");
		return;
	};
	const builtin_handler_t handler = _find_builtin(cmd);
	handler(args);
	return;
};