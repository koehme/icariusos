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
    MAX_DRIVE_LENGTH = 2,
} PathParserDescriptor;

typedef struct PathParser
{
    PathToken curr;
    PathToken prev;
    bool has_error;
} PathParser;

typedef struct PathNode
{
    char identifier[MAX_NAME_LENGTH];
    struct PathNode *next;
} PathNode;

typedef struct PathRootNode
{
    char drive[MAX_DRIVE_LENGTH];
    PathNode *path;
} PathRootNode;

PathRootNode *path_parser_parse(PathParser *self, PathLexer *path_lexer);

#endif