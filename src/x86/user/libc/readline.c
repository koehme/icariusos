#include "readline/readline.h"
#include "history.h"
#include "icarius.h"
#include "kbd.h"
#include "stdio.h"
#include "string.h"
#include "sys/unistd.h"
#include "syscall.h"

#define BUFFER_SIZE 256

static void _flush_line(int pos)
{
	while (pos > 0) {
		write(STDOUT_FILENO, "\b \b", 3);
		pos--;
	};
	return;
};

char* readline(const char* prompt)
{
	if (prompt) {
		write(STDOUT_FILENO, prompt, strlen(prompt));
	};
	size_t pos = 0;
	static char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	uint8_t scancode;

	while (1) {
		if (read(STDIN_FILENO, &scancode, 1) < 0) {
			return NULL;
		};
		const key_event_t ev = translate_sc_into_event(scancode);

		if (!ev.pressed) {
			continue;
		};
		switch (ev.code) {
		case KEY_ENTER: {
			write(STDOUT_FILENO, "\n", 1);
			buffer[pos] = '\0';
			history_add(&icarsh_history, buffer);
			return buffer;
		};
		case KEY_BACKSPACE: {
			if (pos > 0) {
				pos--;
				write(STDOUT_FILENO, "\b \b", 3);
			};
			break;
		};
		case KEY_UP: {
			const char* cmd = history_get_older(&icarsh_history);

			if (cmd) {
				_flush_line(pos);
				strncpy(buffer, cmd, BUFFER_SIZE - 1);
				pos = strlen(buffer);
				write(STDOUT_FILENO, cmd, HISTORY_LINE_MAX);
				break;
			};
			break;
		};
		case KEY_DOWN: {
			const char* cmd = history_get_newer(&icarsh_history);

			if (cmd) {
				_flush_line(pos);
				strncpy(buffer, cmd, BUFFER_SIZE - 1);
				pos = strlen(buffer);
				write(STDOUT_FILENO, cmd, HISTORY_LINE_MAX);
				break;
			};
			break;
		};
		default: {
			if (ev.ascii && pos < BUFFER_SIZE - 1) {
				buffer[pos++] = ev.ascii;
				write(STDOUT_FILENO, &ev.ascii, 1);
			};
			break;
		};
		};
	};
	return NULL;
};