#ifndef MARKCORE_PARSER_H
#define MARKCORE_PARSER_H

#include <stdlib.h>
#include "types.h"

/*
 * Parse a markdown buffer into an AST.
 *
 * Node content/data are spans into `markdown`; that buffer must remain
 * valid until markcore_free_syntax_tree() is called on the returned tree.
 */
MCNode_t *markcore_parse(const char *markdown, size_t len);
void markcore_free_syntax_tree(MCNode_t *root);

// DEBUG ======================================

void markcore_print_tree(MCNode_t *root, int depth);

#endif