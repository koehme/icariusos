/**
 * @file pathparser.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PATHPARSER_H
#define PATHPARSER_H

#include "pathlexer.h"
#include <stdbool.h>

typedef struct pathparser_t {
	pathtoken_t curr;
	pathtoken_t prev;
	bool has_error;
} pathparser_t;

typedef struct pathnode_t {
	char identifier[8];
	struct pathnode_t* next;
} pathnode_t;

typedef struct pathroot_node_t {
	char drive[2];
	pathnode_t* path;
} pathroot_node_t;

pathroot_node_t* path_parser_parse(pathparser_t* self, const char* path);
void path_parser_free(pathroot_node_t* root);

#endif