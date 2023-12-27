/**
 * @file pparser.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pparser.h"
#include "icarius.h"
#include "string.h"

PParser pparser = {
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

static void pparser_handle_syntax_error(PParser *self, const char *message)
{
    printf("Syntax Error occurred! ");

    if (self->prev.type == PT_END)
    {
        printf("at the end.\n");
    }
    else
    {
        printf(message);
    };
    self->has_error = true;
    return;
};

static void pparser_report_error(PParser *self, const char *message)
{
    pparser_handle_syntax_error(self, message);
    return;
};

static bool pparser_check(const PParser *self, const PathType expected)
{
    return self->curr.type == expected;
};

static PathType parser_peek(const PParser *self)
{
    return self->curr.type;
};

static void pparser_advance(PParser *self, PLexer *plexer)
{
    self->prev = self->curr;

    for (;;)
    {
        self->curr = plexer_lex(plexer);

        if (self->curr.type != PT_ERR)
        {
            break;
        };
        pparser_report_error(self, "Oops. Unexpected token.\n");
    };
    return;
};

static bool pparser_match(PParser *self, PLexer *lexer, const PathType expected)
{
    if (pparser_check(self, expected))
    {
        pparser_advance(self, lexer);
        return true;
    };
    return false;
};

static void pparser_eat(PParser *self, PLexer *plexer, const PathType expected, const char *message)
{
    if (pparser_check(self, expected))
    {
        pparser_advance(self, plexer);
        return;
    };
    pparser_report_error(self, message);
    return;
};

PathNode *pparser_parse_dir(PParser *self, PLexer *plexer, PathNode *curr_node)
{
    mcpy(curr_node->identifier, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *pparser_parse_filename(PParser *self, PLexer *plexer, PathNode *curr_node)
{
    size_t bytes = self->prev.len;
    pparser_eat(self, plexer, PT_DOT, "Expect an '.' in an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    bytes += self->prev.len;
    pparser_eat(self, plexer, PT_IDENTIFIER, "Expect an 'identifier' at the end of an filename.");
    mcpy(curr_node->identifier + bytes, self->prev.start, self->prev.len);
    curr_node->next = 0x0;
    return curr_node;
};

PathNode *pparser_parse_entry(PParser *self, PLexer *plexer, PathNode *curr_node)
{
    if (pparser_match(self, plexer, PT_IDENTIFIER))
    {
        curr_node = pparser_parse_dir(self, plexer, curr_node);

        if (pparser_check(self, PT_DOT))
        {
            return pparser_parse_filename(self, plexer, curr_node);
        };
    };
    return curr_node;
};

PathNode *path_parser_parse_entries(PParser *self, PLexer *plexer, PathNode *curr_node)
{
    PathNode *head = 0x0;

    while (pparser_match(self, plexer, PT_SLASH))
    {
        PathNode *new_node = kcalloc(sizeof(PathNode));

        if (!curr_node)
        {
            head = new_node;
            curr_node = head;
        };
        curr_node->next = pparser_parse_entry(self, plexer, new_node);
        curr_node = curr_node->next;
    };
    return head;
};

PathRootNode *pparser_parse_drive(PParser *self, PLexer *plexer)
{
    PathRootNode *root = kcalloc(sizeof(PathRootNode));

    if (pparser_match(self, plexer, PT_LETTER))
    {
        mcpy(root->drive, self->prev.start, 1 * sizeof(char));
        root->drive[1] = '\0';
        pparser_eat(self, plexer, PT_COLON, "Expect an ':' after an drive letter.");
    }
    else
    {
        root->drive[0] = '\0';
    };
    root->path = 0x0;
    return root;
};

PathRootNode *pparser_parse_path(PParser *self, PLexer *plexer)
{
    PathRootNode *root = pparser_parse_drive(self, plexer);
    root->path = path_parser_parse_entries(self, plexer, 0x0);
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
 *            PParserNode = {
 *                .identifier = "bin",
 *                .next = {
 *                    PParserNode = {
 *                        .identifier = "cli.exe",
 *                        .next = { 0x0 }
 *                    }
 *                }
 *            }
 *        }
 *    };
 *
 * @param self A pointer to the PParser instance.
 * @param plexer A pointer to the PLexer instance containing the input to be parsed.
 * @return A pointer to the root node of the parsed file system structure.
 */
PathRootNode *pparser_parse(PParser *self, PLexer *plexer)
{
    PathRootNode *root = 0x0;
    pparser_advance(self, plexer);
    root = pparser_parse_path(self, plexer);
    return root;
};