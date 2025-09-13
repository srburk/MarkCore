#ifndef MARKCORE_PARSER_H
#define MARKCORE_PARSER_H

#include <stdlib.h>
#include "types.h"

// Parse full markdown buffer and return tree
MCNode_t *markcore_parse(char *markdown, size_t len);

// DEBUG ======================================

void markcore_print_tree(MCNode_t *root, int depth);

#endif