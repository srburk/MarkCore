
#ifndef MARKCORE_TYPES_H
#define MARKCORE_TYPES_H

#include <stddef.h> // for size_t

typedef enum {
	ROOT_NODE,
	LINE_NODE,
	TEXT_NODE,
	LINK_NODE,
	HEADER_NODE,
	IMAGE_NODE,
	BOLD_NODE,
	ITALIC_NODE, 
	CODE_INLINE_NODE,
	UNORDERED_LIST_NODE,
	ORDERED_LIST_NODE,
	CODE_BLOCK_NODE,
	BOLD_ITALIC_NODE,
	NODE_TYPE_COUNT
} MCNodeType_e;

typedef struct MCNode {
	MCNodeType_e type;

	// String View (pointer into original source, no copy)
	const char *content;
	size_t content_len;

	struct MCNode **children;
	int child_count;
	int child_capacity;
	
	// type-specific data
	union {
		int header_level;
	};
	
	// String View for extra data (e.g. URL)
	const char *data;
	size_t data_len;
	
} MCNode_t;

static const char *type_labels[NODE_TYPE_COUNT] = {
	[ROOT_NODE] = "Root",
	[HEADER_NODE] = "Header",
	[LINK_NODE] = "Link",
	[IMAGE_NODE] = "Image",
	[LINE_NODE] = "Line",
	[BOLD_NODE] = "Bold",
	[ITALIC_NODE] = "Italic",
	[BOLD_ITALIC_NODE] = "Bold AND Italic",
	[CODE_BLOCK_NODE] = "Code Block",
	[CODE_INLINE_NODE] = "Inline code",
	[UNORDERED_LIST_NODE] = "Unordered list",
	[ORDERED_LIST_NODE] = "Ordered list",
	[TEXT_NODE] = "Text",
};

#endif
