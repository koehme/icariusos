/**
 * @file pparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PPARSER_H
#define PPARSER_H

#include <stdbool.h>
#include "plexer.h"

typedef enum PParserLimits
{
    MAX_NAME_LENGTH = 8,
    MAX_DRIVE_LENGTH = 2,
} PParserLimits;

typedef struct PParser
{
    PathToken curr;
    PathToken prev;
    bool has_error;
} PParser;

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

PathRootNode *pparser_parse(PParser *self, PLexer *plexer);

#endif