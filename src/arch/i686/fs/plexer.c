#include "plexer.h"
#include "string.h"
#include <stdbool.h>

PLexer plexer = {
    .curr = 0x0,
    .start = 0x0,
};

inline static bool plexer_is_eof(const PLexer *self)
{
    return *self->curr == '\0';
};

static char plexer_advance(PLexer *self)
{
    self->curr = self->curr + 1;
    return *(self->curr - 1);
};

static PathToken plexer_create_token(PLexer *self, PathType type)
{
    const PathToken token = {
        .type = type,
        .start = self->start,
        .len = (int)(self->curr - self->start),
    };
    return token;
};

void plexer_init(PLexer *self, const char *path)
{
    self->start = (char *)path;
    self->curr = (char *)path;
    return;
};

inline static char plexer_peek(const PLexer *self)
{
    return *self->curr;
};

static char plexer_peek_next(const PLexer *self)
{
    if (plexer_is_eof(self))
    {
        return '\0';
    };
    return *(self->curr + 1);
};

static PathToken plexer_lex_identifier(PLexer *self)
{
    while (is_alpha(plexer_peek(self)))
    {
        plexer_advance(self);
    };
    return plexer_create_token(self, PT_IDENTIFIER);
};

PathToken plexer_lex(PLexer *self)
{
    self->start = self->curr;

    if (plexer_is_eof(self))
    {
        return plexer_create_token(self, PT_END);
    };

    if (is_alpha(plexer_peek(self)) && plexer_peek_next(self) == ':')
    {
        plexer_advance(self);
        return plexer_create_token(self, PT_LETTER);
    };
    const char ch = plexer_advance(self);

    switch (ch)
    {
    case '/':
    {
        return plexer_create_token(self, PT_SLASH);
    };
    case ':':
    {
        return plexer_create_token(self, PT_COLON);
    };
    case '.':
    {
        return plexer_create_token(self, PT_DOT);
    };
    default:
    {
        if (is_alpha(ch))
        {
            return plexer_lex_identifier(self);
        };
    };
    };
    return plexer_create_token(self, PT_ERR);
};