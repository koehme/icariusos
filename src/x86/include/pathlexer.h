/**
 * @file pathlexer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PATHLEXER_H
#define PATHLEXER_H

#include <stdint.h>

typedef enum pathtype {
	PT_IDENTIFIER,
	PT_LETTER,
	PT_COLON,
	PT_SLASH,
	PT_DOT,
	PT_END,
	PT_ERR,
} pathtype_t;

typedef struct pathtoken {
	pathtype_t type;
	char* start;
	int32_t len;
} pathtoken_t;

typedef struct pathlexer_t {
	char* start;
	char* curr;
} pathlexer_t;

void path_lexer_init(pathlexer_t* self, const char* path);
pathtoken_t path_lexer_lex(pathlexer_t* self);

#endif