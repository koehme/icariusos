/**
 * @file pathparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PATHPARSER_H
#define PATHPARSER_H

#include <stdbool.h>
#include "pathlexer.h"

typedef enum PathParserDescriptor
{
    MAX_NAME_LENGTH = 8,
} PathParserDescriptor;

typedef struct PathParser
{
    char *next;
    char *prev;
    bool has_error;
} PathParser;

typedef struct PathParserNode
{
    char identifier[MAX_NAME_LENGTH];
    struct PathParserNode *next;
} PathParserNode;

typedef struct PathParserRootNode
{
    char drive;
    PathParserNode *path;
} PathParserRootNode;

PathParserRootNode *path_parser_parse(PathParser *self, PathLexer *path_lexer);

#endif