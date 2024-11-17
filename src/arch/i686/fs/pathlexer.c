/**
 * @file pathlexer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathlexer.h"
#include "ctype.h"

#include <stdbool.h>

pathlexer_t path_lexer = {
    .curr = 0x0,
    .start = 0x0,
};

/* PUBLIC API */
void path_lexer_init(pathlexer_t* self, const char* path);
pathtoken_t path_lexer_lex(pathlexer_t* self);

/* INTERNAL API */
inline static bool path_lexer_is_eof(const pathlexer_t* self);
inline static char path_lexer_peek(const pathlexer_t* self);
static char path_lexer_peek_next(const pathlexer_t* self);
static char path_lexer_advance(pathlexer_t* self);
static pathtoken_t path_lexer_create_token(pathlexer_t* self, pathtype_t type);
static pathtoken_t path_lexer_lex_identifier(pathlexer_t* self);

inline static bool path_lexer_is_eof(const pathlexer_t* self) { return *self->curr == '\0'; };

static char path_lexer_advance(pathlexer_t* self)
{
	self->curr = self->curr + 1;
	return *(self->curr - 1);
};

static pathtoken_t path_lexer_create_token(pathlexer_t* self, pathtype_t type)
{
	const pathtoken_t token = {
	    .type = type,
	    .start = self->start,
	    .len = (int32_t)(self->curr - self->start),
	};
	return token;
};

void path_lexer_init(pathlexer_t* self, const char* path)
{
	self->start = (char*)path;
	self->curr = (char*)path;
	return;
};

inline static char path_lexer_peek(const pathlexer_t* self) { return *self->curr; };

static char path_lexer_peek_next(const pathlexer_t* self)
{
	if (path_lexer_is_eof(self)) {
		return '\0';
	};
	return *(self->curr + 1);
};

static pathtoken_t path_lexer_lex_identifier(pathlexer_t* self)
{
	while (isalpha(path_lexer_peek(self))) {
		path_lexer_advance(self);
	};
	return path_lexer_create_token(self, PT_IDENTIFIER);
};

pathtoken_t path_lexer_lex(pathlexer_t* self)
{
	self->start = self->curr;

	if (path_lexer_is_eof(self)) {
		return path_lexer_create_token(self, PT_END);
	};

	if (isalpha(path_lexer_peek(self)) && path_lexer_peek_next(self) == ':') {
		path_lexer_advance(self);
		return path_lexer_create_token(self, PT_LETTER);
	};
	const char ch = path_lexer_advance(self);

	switch (ch) {
	case '/': {
		return path_lexer_create_token(self, PT_SLASH);
	};
	case ':': {
		return path_lexer_create_token(self, PT_COLON);
	};
	case '.': {
		return path_lexer_create_token(self, PT_DOT);
	};
	default: {
		if (isalpha(ch)) {
			return path_lexer_lex_identifier(self);
		};
	};
	};
	return path_lexer_create_token(self, PT_ERR);
};