
#ifndef MARKCORE_TYPES_H
#define MARKCORE_TYPES_H

typedef enum {
	ROOT_NODE,
	LINE_NODE,
	TEXT_NODE,
	LINK_NODE,
	HEADER_NODE,
	IMAGE_NODE,
	BOLD_NODE,
	ITALIC_NODE, 
	UNORDERED_LIST_NODE,
	CODE_BLOCK_NODE,
	BOLD_ITALIC_NODE,
	NODE_TYPE_COUNT
} MCNodeType_e;

typedef struct MCNode {
	MCNodeType_e type;
	char *content;
	struct MCNode **children;
	int child_count;
	int child_capacity;
	
	// type-specific data
	int header_level;
	char *data;
} MCNode_t;

static char *type_labels[NODE_TYPE_COUNT] = {
	[ROOT_NODE] = "Root",
	[HEADER_NODE] = "Header",
	[LINK_NODE] = "Link",
	[IMAGE_NODE] = "Image",
	[LINE_NODE] = "Line",
	[BOLD_NODE] = "Bold",
	[ITALIC_NODE] = "Italic",
	[BOLD_ITALIC_NODE] = "Bold AND Italic",
	[CODE_BLOCK_NODE] = "Code Block",
	[UNORDERED_LIST_NODE] = "Unordered list",
	[TEXT_NODE] = "Text",
};

#endif