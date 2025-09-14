#ifndef MARKCORE_RENDERER_H
#define MARKCORE_RENDERER_H

#include "types.h"
#include "stack.h"

typedef struct Renderer {

	Stack_t *node_stack; // list state
	
	FILE *outfile;

	void (*render_header)(struct Renderer*, int header_level, const char *text);	
	void (*render_text)(struct Renderer*, const char *text);
	void (*render_image)(struct Renderer*, const char *url, const char *alt);
	void (*render_link)(struct Renderer*, const char *url, const char *text);
	
	void (*render_line_end)(struct Renderer*);
	
	void (*render_paragraph_open)(struct Renderer*);
	void (*render_paragraph_close)(struct Renderer*);
	
	void (*render_code_block_open)(struct Renderer*);
	void (*render_code_block_close)(struct Renderer*);
	
	void (*render_code_inline)(struct Renderer*, const char *text);
	
	void (*render_bold_open)(struct Renderer*);
	void (*render_bold_close)(struct Renderer*);
	
	void (*render_italic_open)(struct Renderer*);
	void (*render_italic_close)(struct Renderer*);
	
	void (*render_unordered_list_open)(struct Renderer*);
	void (*render_unordered_list_close)(struct Renderer*);
	void (*render_ordered_list_open)(struct Renderer*);
	void (*render_ordered_list_close)(struct Renderer*);
	
	void (*render_list_item_open)(struct Renderer*);
	void (*render_list_item_close)(struct Renderer*);
} Renderer_t;

void render_syntax_tree(Renderer_t *r, MCNode_t *node);

#endif