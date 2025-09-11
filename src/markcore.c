
#include "markcore.h"
#include "buffer.h"
#include "parser.h"
#include <stdio.h>

#define DYNAMIC_BUFFER_INITIAL_SIZE 1024

// Public ===========================================

char *markcore_render(const char *markdown, size_t length) {
	Buffer_t buffer = {};
	if (buffer_init(&buffer, DYNAMIC_BUFFER_INITIAL_SIZE) == -1) {
		fprintf(stderr, "Markcore error, couldn't initialize dynamic buffer\n");
		return NULL;
	}
	
	
	MarkCoreNode_t *node = markcore_parse(markdown, length);
	markcore_print_tree(node, 0);
	
	return buffer.data;
}

// size_t markcore_render_to_buffer(const char *markdown,
// 								 char *write_buffer,
// 								 size_t buffer_size);
// 
// size_t markcore_render_to_file(const char *markdown,
// 								FILE *out_file);