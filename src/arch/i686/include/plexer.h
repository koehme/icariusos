/**
 * @file plexer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PLEXER_H
#define PLEXER_H

typedef enum PathType
{
    PT_IDENTIFIER,
    PT_LETTER,
    PT_COLON,
    PT_SLASH,
    PT_DOT,
    PT_END,
    PT_ERR,
} PathType;

typedef struct PathToken
{
    PathType type;
    char *start;
    int len;
} PathToken;

typedef struct PLexer
{
    char *start;
    char *curr;
} PLexer;

void plexer_init(PLexer *self, const char *path);
PathToken plexer_lex(PLexer *self);

#endif