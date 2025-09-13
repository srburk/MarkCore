#ifndef MARKCORE_PARSER_H
#define MARKCORE_PARSER_H

#include <stdlib.h>

typedef enum {
	ROOT_NODE,
	LINE_NODE,
	TEXT_NODE,
	LINK_NODE,
	HEADER_NODE,
	IMAGE_NODE,
	BOLD_NODE,
	ITALIC_NODE,
	BOLD_ITALIC_NODE,
	NODE_TYPE_COUNT
} MarkCoreNodeType_t;

typedef struct MarkCoreNode {
	MarkCoreNodeType_t type;
	char *content;
	struct MarkCoreNode **children;
	int child_count;
	int child_capacity;
	
	// type-specific data
	union {
        int header_level;
        char *data;
    };
} MarkCoreNode_t;

/*
 * Captures dependencies across lines
 */
typedef struct {
// 	position
//  in code block
// in list (ul or ol)
// blockquote
} MarkCoreParserState_t;

// Parse full markdown buffer and return tree (internally this should probably be markcore_parse_line with automatic state management)
MarkCoreNode_t *markcore_parse(char *markdown, size_t len);

// Parse line (requires manual state tracking)
MarkCoreNode_t *markcore_parse_line(char *markdown, size_t len);

// DEBUG ======================================

void markcore_print_tree(MarkCoreNode_t *root, int depth);

#endif