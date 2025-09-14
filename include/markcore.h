

#ifndef MARKCORE_H
#define MARKCORE_H

#include <stdio.h>

// typedef enum {
// 	enable_code_blocks = 1 << 0,
// } MarkCoreOptions_e;

/*
Returns dynamically allocated array with contents. Please free()
*/
// char *markcore_render(const char *markdown, size_t length);

// size_t markcore_render_to_buffer(const char *markdown,
// 							     size_t length,
// 								 char *write_buffer,
// 								 size_t buffer_size);

size_t markcore_render_to_file(const char *markdown,
							   size_t length,
							   FILE *out_file);

#endif