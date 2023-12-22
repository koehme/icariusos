/**
 * @file pparser.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pathparser.h"
#include "icarius.h"

/*
GRAMMAR

path                -> drive ':' '/' directory
directory           -> identifier directoryTail
directory_tail      -> '/' identifier directoryTail | ε

identifier          -> 'a' | 'b' | ... | 'z'
drive               -> 'A' | 'B' | ... | 'Z'
*/

static PathParserNode *path_parser_parse_path(PathParser *self, PathParserRootNode *root_node, PathLexer *path_lexer)
{
    path_parser_advance(self, path_lexer);

    while (!parser_match(self, path_lexer, PT_END))
    {
    };
};

/**
 * @brief Parses a path and constructs a PParserRootNode representing the file system path structure.
 * Takes a path as input and constructs a PParserRootNode, which represents
 * the file system structure based on the provided path. The expected format of the path is
 * similar to the example below:
 * @example
 *     root_node = {
 *        .drive = 'A', .path = {
 *            PathParsrNode = {
 *                .identifier = "bin",
 *                .next = {
 *                    PathParsrNode = {
 *                        .identifier = "cli",
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
PathParserRootNode *path_parser_parse(PathParser *self, PathLexer *path_lexer)
{
    PathParserRootNode *root_node = kcalloc(sizeof(PathParserRootNode));
    // Pass a double pointer to not modify the original root_node to path_parser_parse_path
    return root_node;
};