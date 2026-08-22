#ifndef MARKCORE_TYPES_H
#define MARKCORE_TYPES_H

#include <stddef.h>

typedef struct MCArena MCArena_t;

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

/* View into the original markdown buffer. Not NUL-terminated; not owned. */
typedef struct {
	const char *ptr;
	size_t len;
} MCSpan_t;

typedef struct MCNode {
	MCNodeType_e type;
	MCSpan_t content;
	struct MCNode **children;
	int child_count;
	int child_capacity;

	/* type-specific: heading level, link/image URL, or root arena */
	union {
		int header_level;
		MCSpan_t data;
		MCArena_t *arena;
	};
} MCNode_t;

static inline MCSpan_t mc_span_range(const char *start, const char *end)
{
	MCSpan_t s;
	if (!start || !end || end < start) {
		s.ptr = NULL;
		s.len = 0;
		return s;
	}
	s.ptr = start;
	s.len = (size_t)(end - start);
	return s;
}

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
	[CODE_INLINE_NODE] = "Inline code",
	[UNORDERED_LIST_NODE] = "Unordered list",
	[ORDERED_LIST_NODE] = "Ordered list",
	[TEXT_NODE] = "Text",
};

#endif
