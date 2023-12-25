/**
 * @file pparser.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathparser.h"
#include "icarius.h"

PathParser pparser = {
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
    kprint("Syntax Error occurred! ");

    if (self->prev.type == PT_END)
    {
        kprint("at the end.\n");
    }
    else
    {
        kprint(message);
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

static PathType parser_peek(const PathParser *self)
{
    return self->curr.type;
};

static void path_parser_advance(PathParser *self, PathLexer *path_lexer)
{
    self->prev = self->curr;

    for (;;)
    {
        self->curr = path_lexer_lex(path_lexer);

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

static void path_parser_eat(PathParser *self, PathLexer *path_lexer, const PathType expected, const char *message)
{
    if (path_parser_check(self, expected))
    {
        path_parser_advance(self, path_lexer);
        return;
    };
    path_parser_report_error(self, message);
    return;
};

PathNode *path_parser_parse_directory(PathParser *self, PathLexer *path_lexer, PathNode *curr_node)
{
    mcpy(curr_node->identifier, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *path_parser_parse_filename(PathParser *self, PathLexer *path_lexer, PathNode *curr_node)
{
    size_t bytes = self->prev.len;
    path_parser_eat(self, path_lexer, PT_DOT, "Expect an '.' in an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    bytes += self->prev.len;
    path_parser_eat(self, path_lexer, PT_IDENTIFIER, "Expect an 'identifier' at the end of an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *path_parser_parse_entry(PathParser *self, PathLexer *path_lexer, PathNode *curr_node)
{
    if (path_parser_match(self, path_lexer, PT_IDENTIFIER))
    {
        curr_node = path_parser_parse_directory(self, path_lexer, curr_node);

        if (path_parser_check(self, PT_DOT))
        {
            return path_parser_parse_filename(self, path_lexer, curr_node);
        };
    };
    return curr_node;
};

PathNode *path_parser_parse_entries(PathParser *self, PathLexer *path_lexer, PathNode *curr_node)
{
    PathNode *head = 0x0;

    while (path_parser_match(self, path_lexer, PT_SLASH))
    {
        PathNode *new_node = kcalloc(sizeof(PathNode));
        // Special case if curr_node is empty, it was  the first call so we set head to new_node,
        // so we have a ref to the head, which can we use to build the full path
        if (!curr_node)
        {
            head = new_node;
            curr_node = head;
        };
        curr_node->next = path_parser_parse_entry(self, path_lexer, new_node);
        curr_node = curr_node->next;
    };
    return head;
};

PathRootNode *path_parser_parse_drive(PathParser *self, PathLexer *path_lexer)
{
    PathRootNode *root = kcalloc(sizeof(PathRootNode));
    path_parser_eat(self, path_lexer, PT_LETTER, "Expect an letter like 'A' for an drive.");
    mcpy(root->drive, self->prev.start, 1 * sizeof(char));
    root->drive[1] = '\0';
    root->path = 0x0;
    path_parser_eat(self, path_lexer, PT_COLON, "Expect an ':' after an drive letter.");
    return root;
};

// path        -> drive ':' '/' entries
PathRootNode *path_parser_parse_path(PathParser *self, PathLexer *path_lexer)
{
    PathRootNode *root = path_parser_parse_drive(self, path_lexer);
    root->path = path_parser_parse_entries(self, path_lexer, 0x0);
    return root;
};

/**
 * @brief Parses a path and constructs a PParserRootNode representing the file system path structure.
 * Takes a path as input and constructs a PParserRootNode, which represents
 * the file system structure based on the provided path. The expected format of the path is
 * * the file system structure based on the provided path. The expected format of the path is
 * similar to the example below:
 * @example
 *      path = "A:/bin/cli.exe"
 *     root_node = {
 *        .drive = 'A', .path = {
 *            PathParserNode = {
 *                .identifier = "bin",
 *                .next = {
 *                    PathParserNode = {
 *                        .identifier = "cli.exe",
 *                        .next = { 0x0 }
 *                    }
 *                }
 *            }
 *        }
 *    };
 *
 * @param self A pointer to the PParser instance.
 * @param path_lexer A pointer to the PLexer instance containing the input to be parsed.
 * @return A pointer to the root node of the parsed file system structure.
 */
PathRootNode *path_parser_parse(PathParser *self, PathLexer *path_lexer)
{
    path_parser_advance(self, path_lexer);
    return path_parser_parse_path(self, path_lexer);
};