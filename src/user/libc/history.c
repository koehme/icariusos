#include "history.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>

history_t icarsh_history = {};

void history_add(history_t* self, const char* line)
{
	const size_t len = strlen(line);

	if (!line || !self || len == 0) {
		return;
	};
	// Check if last cmd is the same like line
	if (self->count > 0) {
		const int last_line = self->head - 1;
		const int i = (last_line + HISTORY_SIZE) % HISTORY_SIZE;
		const bool is_same_line = strcmp(self->entries[i], line) == 0 ? true : false;

		if (is_same_line) {
			return;
		};
	};
	// Add the line
	strncpy(self->entries[self->head], line, HISTORY_LINE_MAX - 1);
	self->entries[self->head][HISTORY_LINE_MAX - 1] = '\0';
	// Advance and wrap around if needed
	self->head = (self->head + 1) % HISTORY_SIZE;

	if (self->count < HISTORY_SIZE) {
		self->count++;
	};
	// point to empty line " "
	self->cursor = self->head;
	return;
};

const char* history_get_older(history_t* self)
{
	if (!self || self->count == 0) {
		return 0x0;
	};
	const int oldest_index = ((self->head - self->count) + HISTORY_SIZE) % HISTORY_SIZE;

	if (self->cursor == oldest_index) {
		return self->entries[oldest_index];
	};
	const int older_index = ((self->cursor) - 1 + HISTORY_SIZE) % HISTORY_SIZE;
	self->cursor = older_index;
	return self->entries[self->cursor];
};

const char* history_get_newer(history_t* self)
{
	if (!self || self->count == 0) {
		return 0x0;
	};
	if (self->cursor == self->head) {
		return "";
	};
	const int next_index = (self->cursor + 1) % HISTORY_SIZE;
	self->cursor = next_index;

	if (next_index == self->head) {
		return "";
	};
	const char* line = self->entries[next_index];
	return line;
};

void history_dump(history_t* self)
{
	if (!self || self->count == 0) {
		printf("History is empty.\n");
		return;
	};
	printf("----[ History Dump ]----\n");
	printf("Head: %d, Count: %d, Cursor: %d\n", self->head, self->count, self->cursor);
	const int oldest_index = (self->head - self->count + HISTORY_SIZE) % HISTORY_SIZE;

	for (int i = 0; i < self->count; i++) {
		const int index = (oldest_index + i) % HISTORY_SIZE;
		printf("[%d]%s%s\n", index, "   ", self->entries[index]);
	};
	printf("------------------------\n");
	return;
};