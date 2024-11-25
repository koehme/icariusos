/**
 * @file pathparser.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathparser.h"
#include "kernel.h"
#include "string.h"

pathparser_t path_parser = {
    .curr = {},
    .prev = {},
    .has_error = false,
};

/* PUBLIC API */
pathroot_node_t* path_parser_parse(pathparser_t* self, const char* path);
void path_parser_free(pathroot_node_t* root);

/* INTERNAL API */
static void path_parser_handle_syntax_error(pathparser_t* self, const char* message);
static void path_parser_report_error(pathparser_t* self, const char* message);
static bool path_parser_check(const pathparser_t* self, const pathtype_t expected);
static pathtype_t path_parser_peek(const pathparser_t* self);
static void path_parser_advance(pathparser_t* self, pathlexer_t* lexer);
static bool path_parser_match(pathparser_t* self, pathlexer_t* lexer, const pathtype_t expected);
static void path_parser_eat(pathparser_t* self, pathlexer_t* lexer, const pathtype_t expected, const char* message);
pathnode_t* path_parser_parse_dir(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node);
pathnode_t* path_parser_parse_filename(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node);
pathnode_t* path_parser_parse_entry(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node);
pathnode_t* path_parser_parse_entries(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node);
pathroot_node_t* path_parser_parse_drive(pathparser_t* self, pathlexer_t* lexer);
pathroot_node_t* path_parser_parse_path(pathparser_t* self, pathlexer_t* lexer);
static void free_node(pathnode_t* curr_node);

/*
path        -> drive ':' entries
drive       -> letter

entries     -> ( '/' entry )*
entry       -> directory | filename

directory   -> identifier
filename    -> identifier '.' identifier

identifier  -> ( 'a' - 'z' )*
letter      -> ( 'A' - 'Z' )
*/

static void path_parser_handle_syntax_error(pathparser_t* self, const char* message)
{
	printf("Syntax Error occurred! ");

	if (self->prev.type == PT_END) {
		printf("at the end.\n");
	} else {
		printf(message);
	};
	self->has_error = true;
	return;
};

static void path_parser_report_error(pathparser_t* self, const char* message)
{
	path_parser_handle_syntax_error(self, message);
	return;
};

static bool path_parser_check(const pathparser_t* self, const pathtype_t expected) { return self->curr.type == expected; };

static pathtype_t path_parser_peek(const pathparser_t* self) { return self->curr.type; };

static void path_parser_advance(pathparser_t* self, pathlexer_t* lexer)
{
	self->prev = self->curr;

	for (;;) {
		self->curr = path_lexer_lex(lexer);

		if (self->curr.type != PT_ERR) {
			break;
		};
		path_parser_report_error(self, "Oops. Unexpected token.\n");
	};
	return;
};

static bool path_parser_match(pathparser_t* self, pathlexer_t* lexer, const pathtype_t expected)
{
	if (path_parser_check(self, expected)) {
		path_parser_advance(self, lexer);
		return true;
	};
	return false;
};

static void path_parser_eat(pathparser_t* self, pathlexer_t* lexer, const pathtype_t expected, const char* message)
{
	if (path_parser_check(self, expected)) {
		path_parser_advance(self, lexer);
		return;
	};
	path_parser_report_error(self, message);
	return;
};

pathnode_t* path_parser_parse_dir(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node)
{
	memcpy(curr_node->identifier, self->prev.start, self->prev.len);
	curr_node->next = 0x0;
	return curr_node;
};

pathnode_t* path_parser_parse_filename(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node)
{
	size_t bytes = self->prev.len;
	path_parser_eat(self, lexer, PT_DOT, "Expect an '.' in an filename.");
	memcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
	bytes += self->prev.len;
	path_parser_eat(self, lexer, PT_IDENTIFIER, "Expect an 'identifier' at the end of an filename.");
	memcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
	curr_node->next = 0x0;
	return curr_node;
};

pathnode_t* path_parser_parse_entry(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node)
{
	if (path_parser_match(self, lexer, PT_IDENTIFIER)) {
		curr_node = path_parser_parse_dir(self, lexer, curr_node);

		if (path_parser_check(self, PT_DOT)) {
			return path_parser_parse_filename(self, lexer, curr_node);
		};
	};
	return curr_node;
};

pathnode_t* path_parser_parse_entries(pathparser_t* self, pathlexer_t* lexer, pathnode_t* curr_node)
{
	pathnode_t* head = 0x0;
	pathnode_t* prev = 0x0;

	while (path_parser_match(self, lexer, PT_SLASH)) {
		pathnode_t* new_node = kzalloc(sizeof(pathnode_t));

		if (!head) {
			head = new_node;
		} else {
			prev->next = new_node;
		};
		prev = new_node;
		curr_node = path_parser_parse_entry(self, lexer, new_node);
	};
	return head;
};

pathroot_node_t* path_parser_parse_drive(pathparser_t* self, pathlexer_t* lexer)
{
	pathroot_node_t* root = kzalloc(sizeof(pathroot_node_t));

	if (path_parser_match(self, lexer, PT_LETTER)) {
		memcpy(root->drive, self->prev.start, 1 * sizeof(char));
		root->drive[1] = '\0';
		path_parser_eat(self, lexer, PT_COLON, "Expect an ':' after an drive letter.");
	} else {
		root->drive[0] = '\0';
	};
	root->path = 0x0;
	return root;
};

pathroot_node_t* path_parser_parse_path(pathparser_t* self, pathlexer_t* lexer)
{
	pathroot_node_t* root = path_parser_parse_drive(self, lexer);
	root->path = path_parser_parse_entries(self, lexer, 0x0);
	return root;
};

// Creates a data structure representing a given file system path for easy traversal of the path
pathroot_node_t* path_parser_parse(pathparser_t* self, const char* path)
{
	pathlexer_t lexer = {};
	path_lexer_init(&lexer, path);
	pathroot_node_t* root = 0x0;
	path_parser_advance(self, &lexer);
	root = path_parser_parse_path(self, &lexer);
	return root;
};

static void free_node(pathnode_t* curr_node)
{
	if (curr_node == 0x0) {
		return;
	};
	free_node(curr_node->next);
	kfree(curr_node);
};

void path_parser_free(pathroot_node_t* root)
{
	if (root == 0x0) {
		return;
	};
	free_node(root->path);
	kfree(root);
	return;
};