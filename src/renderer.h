#ifndef MARKCORE_RENDERER_H
#define MARKCORE_RENDERER_H

#include <stdio.h>

#include "types.h"
#include "stack.h"

typedef struct Renderer {

	Stack_t *node_stack; // list state
	
	FILE *outfile;

	size_t (*render_header)(struct Renderer*, int header_level, const char *text);	
	size_t (*render_text)(struct Renderer*, const char *text);
	size_t (*render_image)(struct Renderer*, const char *url, const char *alt);
	size_t (*render_link)(struct Renderer*, const char *url, const char *text);
	
	size_t (*render_line_end)(struct Renderer*);
	
	size_t (*render_paragraph_open)(struct Renderer*);
	size_t (*render_paragraph_close)(struct Renderer*);
	
	size_t (*render_code_block_open)(struct Renderer*);
	size_t (*render_code_block_close)(struct Renderer*);
	size_t (*render_code_block_line)(struct Renderer*, const char *text);
	
	size_t (*render_code_inline)(struct Renderer*, const char *text);
	
	size_t (*render_bold_open)(struct Renderer*);
	size_t (*render_bold_close)(struct Renderer*);
	
	size_t (*render_italic_open)(struct Renderer*);
	size_t (*render_italic_close)(struct Renderer*);
	
	size_t (*render_unordered_list_open)(struct Renderer*);
	size_t (*render_unordered_list_close)(struct Renderer*);
	size_t (*render_ordered_list_open)(struct Renderer*);
	size_t (*render_ordered_list_close)(struct Renderer*);
	
	size_t (*render_list_item_open)(struct Renderer*);
	size_t (*render_list_item_close)(struct Renderer*);
} Renderer_t;

size_t render_syntax_tree(Renderer_t *r, MCNode_t *node);
void renderer_destroy(Renderer_t *r); // clean up stack

#endif