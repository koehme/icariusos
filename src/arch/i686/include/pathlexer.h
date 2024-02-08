/**
 * @file pathlexer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PATHLEXER_H
#define PATHLEXER_H

#include <stdint.h>

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
    int32_t len;
} PathToken;

typedef struct PathLexer
{
    char *start;
    char *curr;
} PathLexer;

void path_lexer_init(PathLexer *self, const char *path);
PathToken path_lexer_lex(PathLexer *self);

#endif