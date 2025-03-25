#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 16
#define HISTORY_LINE_MAX 256

typedef struct history {
	char entries[HISTORY_SIZE][HISTORY_LINE_MAX];
	int head;   // Points to the next free slot
	int count;  // entries in history
	int cursor; // direction in the history for ↑/↓ navigation
} history_t;

extern history_t icarsh_history;

void history_add(history_t* self, const char* line);
const char* history_get_older(history_t* self);
const char* history_get_newer(history_t* self);
void history_dump(history_t* self);

#endif