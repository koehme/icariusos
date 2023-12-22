/**
 * @file pparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PPARSER_H
#define PPARSER_H

#include <stdbool.h>
#include "plexer.h"

typedef enum PParserDescriptor
{
    MAX_NAME_LENGTH = 8,
} PParserDescriptor;

typedef struct PParser
{
    char *next;
    char *prev;
    bool has_error;
} PParser;

typedef struct PParserRootNode
{
    char drive;
    PParserRootNode *path;
} PParserRootNode;

typedef struct PParserPathNode
{
    char identifier[MAX_NAME_LENGTH];
    PParserPathNode *next;
} PParserPathNode;

PParserRootNode *pparser_parse(PParser *self, PLexer *plexer);

#endif