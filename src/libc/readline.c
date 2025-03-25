#include "readline/readline.h"
#include "history.h"
#include "icarius.h"
#include "kbd.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define BUFFER_SIZE 256

static void _flush_line(int pos)
{
	while (pos > 0) {
		write(1, "\b \b", 4);
		pos--;
	};
	return;
};

char* readline(const char* prompt)
{
	if (prompt) {
		write(1, prompt, strlen(prompt));
	};
	size_t pos = 0;
	static char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	uint8_t scancode;

	while (1) {
		if (read(0, &scancode, 1) <= 0) {
			return 0x0;
		};
		const key_event_t ev = translate_sc_into_event(scancode);

		if (!ev.pressed) {
			continue;
		};
		switch (ev.code) {
		case KEY_ENTER: {
			write(1, "\n", 1);
			buffer[pos] = '\0';
			history_add(&icarsh_history, buffer);
			return buffer;
		};
		case KEY_BACKSPACE: {
			if (pos > 0) {
				pos--;
				write(1, "\b \b", 3);
			};
			break;
		};
		case KEY_UP: {
			const char* cmd = history_get_older(&icarsh_history);

			if (cmd) {
				_flush_line(pos);
			};
			strncpy(buffer, cmd, BUFFER_SIZE - 1);
			pos = strlen(buffer);
			write(1, cmd, HISTORY_LINE_MAX);
			break;
		};
		case KEY_DOWN: {
			const char* cmd = history_get_newer(&icarsh_history);

			if (cmd) {
				_flush_line(pos);
			};
			strncpy(buffer, cmd, BUFFER_SIZE - 1);
			pos = strlen(buffer);
			write(1, cmd, HISTORY_LINE_MAX);
			break;
		};
		default: {
			if (ev.ascii && pos < BUFFER_SIZE - 1) {
				buffer[pos++] = ev.ascii;
				write(1, &ev.ascii, 1);
			};
			break;
		};
		};
	};
	return 0x0;
};