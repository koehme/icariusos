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

/**
 * @brief Parses a path and constructs a PParserRootNode representing the file system path structure.
 * Takes a path as input and constructs a PParserRootNode, which represents
 * the file system structure based on the provided path. The expected format of the path is
 * similar to the example below:
 * @example
 *    PParserRootNode root_node = {
 *        .drive = 'A', .path = {
 *            PParserPathNode = {
 *                .identifier = "bin",
 *                .next = {
 *                    PParserPathNode = {
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
PathParserRootNode *pparser_parse(PathParser *self, PathLexer *path_lexer)
{
    PathParserRootNode *root_node = kcalloc(sizeof(PathParserRootNode));
    return root_node;
};