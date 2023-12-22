/**
 * @file plexer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PLEXER_H
#define PLEXER_H

typedef struct PToken
{
    PTokenType type;
    char *start;
    int len;
} PToken;

typedef struct PLexer
{
    char *start;
    char *curr;
} PLexer;

typedef enum PTokenType
{
    TT_DIRECTORY,
    TT_DIRECTORY_TAIL,
    TT_IDENTIFIER,
    TT_DRIVE,
    TT_COLON,
    TT_SLASH,
    TT_END
} PTokenType;

void plexer_init(PLexer *self, const char *path);
PToken plexer_lex(PLexer *self);

#endif