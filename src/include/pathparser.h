/**
 * @file pathparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PATHPARSER_H
#define PATHPARSER_H

#include <stdbool.h>
#include "pathlexer.h"

typedef struct PathParser
{
    PathToken curr;
    PathToken prev;
    bool has_error;
} PathParser;

typedef struct PathNode
{
    char identifier[8];
    struct PathNode *next;
} PathNode;

typedef struct PathRootNode
{
    char drive[2];
    PathNode *path;
} PathRootNode;

PathRootNode *path_parser_parse(PathParser *self, const char *path);
void path_parser_free(PathRootNode *root);

#endif