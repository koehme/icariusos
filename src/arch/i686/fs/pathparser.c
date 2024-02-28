/**
 * @file pathparser.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathparser.h"
#include "string.h"
#include "kernel.h"

PathParser path_parser = {
    .curr = {},
    .prev = {},
    .has_error = false,
};

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

static void path_parser_handle_syntax_error(PathParser *self, const char *message)
{
    kprtf("Syntax Error occurred! ");

    if (self->prev.type == PT_END)
    {
        kprtf("at the end.\n");
    }
    else
    {
        kprtf(message);
    };
    self->has_error = true;
    return;
};

static void path_parser_report_error(PathParser *self, const char *message)
{
    path_parser_handle_syntax_error(self, message);
    return;
};

static bool path_parser_check(const PathParser *self, const PathType expected)
{
    return self->curr.type == expected;
};

static PathType path_parser_peek(const PathParser *self)
{
    return self->curr.type;
};

static void path_parser_advance(PathParser *self, PathLexer *lexer)
{
    self->prev = self->curr;

    for (;;)
    {
        self->curr = path_lexer_lex(lexer);

        if (self->curr.type != PT_ERR)
        {
            break;
        };
        path_parser_report_error(self, "Oops. Unexpected token.\n");
    };
    return;
};

static bool path_parser_match(PathParser *self, PathLexer *lexer, const PathType expected)
{
    if (path_parser_check(self, expected))
    {
        path_parser_advance(self, lexer);
        return true;
    };
    return false;
};

static void path_parser_eat(PathParser *self, PathLexer *lexer, const PathType expected, const char *message)
{
    if (path_parser_check(self, expected))
    {
        path_parser_advance(self, lexer);
        return;
    };
    path_parser_report_error(self, message);
    return;
};

PathNode *path_parser_parse_dir(PathParser *self, PathLexer *lexer, PathNode *curr_node)
{
    mcpy(curr_node->identifier, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *path_parser_parse_filename(PathParser *self, PathLexer *lexer, PathNode *curr_node)
{
    size_t bytes = self->prev.len;
    path_parser_eat(self, lexer, PT_DOT, "Expect an '.' in an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    bytes += self->prev.len;
    path_parser_eat(self, lexer, PT_IDENTIFIER, "Expect an 'identifier' at the end of an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *path_parser_parse_entry(PathParser *self, PathLexer *lexer, PathNode *curr_node)
{
    if (path_parser_match(self, lexer, PT_IDENTIFIER))
    {
        curr_node = path_parser_parse_dir(self, lexer, curr_node);

        if (path_parser_check(self, PT_DOT))
        {
            return path_parser_parse_filename(self, lexer, curr_node);
        };
    };
    return curr_node;
};

PathNode *path_parser_parse_entries(PathParser *self, PathLexer *lexer, PathNode *curr_node)
{
    PathNode *head = 0x0;
    PathNode *prev = 0x0;

    while (path_parser_match(self, lexer, PT_SLASH))
    {
        PathNode *new_node = kcalloc(sizeof(PathNode));

        if (!head)
        {
            head = new_node;
        }
        else
        {
            prev->next = new_node;
        };
        prev = new_node;
        curr_node = path_parser_parse_entry(self, lexer, new_node);
    };
    return head;
};

PathRootNode *path_parser_parse_drive(PathParser *self, PathLexer *lexer)
{
    PathRootNode *root = kcalloc(sizeof(PathRootNode));

    if (path_parser_match(self, lexer, PT_LETTER))
    {
        mcpy(root->drive, self->prev.start, 1 * sizeof(char));
        root->drive[1] = '\0';
        path_parser_eat(self, lexer, PT_COLON, "Expect an ':' after an drive letter.");
    }
    else
    {
        root->drive[0] = '\0';
    };
    root->path = 0x0;
    return root;
};

PathRootNode *path_parser_parse_path(PathParser *self, PathLexer *lexer)
{
    PathRootNode *root = path_parser_parse_drive(self, lexer);
    root->path = path_parser_parse_entries(self, lexer, 0x0);
    return root;
};

// Creates a data structure representing a given file system path for easy traversal of the path
PathRootNode *path_parser_parse(PathParser *self, const char *path)
{
    PathLexer lexer = {};
    path_lexer_init(&lexer, path);
    PathRootNode *root = 0x0;
    path_parser_advance(self, &lexer);
    root = path_parser_parse_path(self, &lexer);
    return root;
};

static void free_node(PathNode *curr_node)
{
    if (curr_node == 0x0)
    {
        return;
    };
    free_node(curr_node->next);
    kfree(curr_node);
};

void path_parser_free(PathRootNode *root)
{
    if (root == 0x0)
    {
        return;
    };
    free_node(root->path);
    kfree(root);
    return;
};