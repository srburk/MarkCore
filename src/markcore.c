
#include "markcore.h"
#include "parser.h"
#include "types.h"

#include "renderers/html_renderer.h"

#include <stdio.h>

#define DYNAMIC_BUFFER_INITIAL_SIZE 1024

// Public ===========================================

size_t markcore_render_to_file(const char *markdown, size_t length, FILE *out_file) {

	if (!markdown || !out_file) return 0;
	
	MCNode_t *node = markcore_parse(markdown, length);
// 	markcore_print_tree(node, 0);
    
	Renderer_t html_renderer = create_html_renderer(out_file);
	render_syntax_tree(&html_renderer, node);
	
	markcore_free_syntax_tree(node);
}

// size_t markcore_render_to_buffer(const char *markdown,
// 								 char *write_buffer,
// 								 size_t buffer_size);
// 
// size_t markcore_render_to_file(const char *markdown,
// 								FILE *out_file);