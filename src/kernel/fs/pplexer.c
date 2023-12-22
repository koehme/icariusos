#include "plexer.h"
#include <stdbool.h>

inline static bool plexer_is_eof(const PLexer *self)
{
    return *self->curr == '\0';
};

static PToken lexer_create_token(PLexer *self, PTokenType type)
{
    return (PToken){
        .type = type,
        .start = self->start,
        .len = (int)(self->curr - self->start),
    };
};

void plexer_init(PLexer *self, const char *path)
{
    self->start = (char *)path;
    self->curr = (char *)path;
    return;
};

PToken plexer_lex(PLexer *self)
{
    self->start = self->curr;

    if (plexer_is_eof(self))
    {
        return lexer_create_token(self, TT_END);
    };
    if (*self->curr == ':')
    {
        self->curr++;
        return lexer_create_token(self, TT_COLON);
    }
    return;
};