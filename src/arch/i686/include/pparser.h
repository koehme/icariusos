/**
 * @file pparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PPARSER_H
#define PPARSER_H

#include <stdbool.h>
#include "plexer.h"

typedef struct PParser
{
    PathToken curr;
    PathToken prev;
    bool has_error;
} PParser;

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

PathRootNode *pparser_parse(PParser *self, PLexer *plexer);

#endif