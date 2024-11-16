/**
 * @file pathlexer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathlexer.h"
#include "ctype.h"

#include <stdbool.h>

PathLexer path_lexer = {
    .curr = 0x0,
    .start = 0x0,
};

/* PUBLIC API */
void path_lexer_init(PathLexer* self, const char* path);
PathToken path_lexer_lex(PathLexer* self);

/* INTERNAL API */
inline static bool path_lexer_is_eof(const PathLexer* self);
inline static char path_lexer_peek(const PathLexer* self);
static char path_lexer_peek_next(const PathLexer* self);
static char path_lexer_advance(PathLexer* self);
static PathToken path_lexer_create_token(PathLexer* self, PathType type);
static PathToken path_lexer_lex_identifier(PathLexer* self);

inline static bool path_lexer_is_eof(const PathLexer* self) { return *self->curr == '\0'; };

static char path_lexer_advance(PathLexer* self)
{
	self->curr = self->curr + 1;
	return *(self->curr - 1);
};

static PathToken path_lexer_create_token(PathLexer* self, PathType type)
{
	const PathToken token = {
	    .type = type,
	    .start = self->start,
	    .len = (int32_t)(self->curr - self->start),
	};
	return token;
};

void path_lexer_init(PathLexer* self, const char* path)
{
	self->start = (char*)path;
	self->curr = (char*)path;
	return;
};

inline static char path_lexer_peek(const PathLexer* self) { return *self->curr; };

static char path_lexer_peek_next(const PathLexer* self)
{
	if (path_lexer_is_eof(self)) {
		return '\0';
	};
	return *(self->curr + 1);
};

static PathToken path_lexer_lex_identifier(PathLexer* self)
{
	while (isalpha(path_lexer_peek(self))) {
		path_lexer_advance(self);
	};
	return path_lexer_create_token(self, PT_IDENTIFIER);
};

PathToken path_lexer_lex(PathLexer* self)
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