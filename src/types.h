
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
	union {
        int header_level;
        char *data;
    };
} MCNode_t;

#endif