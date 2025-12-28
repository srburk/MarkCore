
#include "parser.h"
#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define INITIAL_CHILD_CAPACITY 1

static Stack_t *node_stack;

// Forward declaration ======================================================

static void markcore_parse_line(const char *markdown, size_t start_idx, size_t end_idx);

static MCNode_t *markcore_parse_image(const char *p, const char *limit);

static void markcore_parse_inline_range(const char *start, const char *end);

static MCNode_t *markcore_parse_link(const char **p_ptr, const char *limit);
static MCNode_t *markcore_parse_italics_bold(const char **p_ptr, const char *limit);

static void flush_text(const char *start, const char *end);

static const char *seek_next_char(const char *p, const char *limit, const char c);

// Tree functions

static MCNode_t *create_node(MCNodeType_e type, const char *content, size_t len) {
	MCNode_t *node = malloc(sizeof(MCNode_t));
    if (!node) return NULL;
    node->type = type;
    node->content = content; // Store pointer only
    node->content_len = len;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
	node->data = NULL;
	node->data_len = 0;
    return node;
}

static void add_child_node(MCNode_t *parent, MCNode_t *child) {
	if (!parent || !child) return;
	
	if (!parent->children) {
		// initialize
		parent->children = malloc(sizeof(MCNode_t *) * INITIAL_CHILD_CAPACITY);
		parent->child_capacity = INITIAL_CHILD_CAPACITY;
	}
	
	if (parent->child_count + 1 > parent->child_capacity) {
		size_t new_capacity = parent->child_capacity * 2;
		MCNode_t **new_children = realloc(parent->children, sizeof(MCNode_t *) * new_capacity);
		if (!new_children) {
			fprintf(stderr, "Failed to realloc children\n");
			return;
		}
		parent->children = new_children;
		parent->child_capacity = new_capacity;
	}
	parent->children[parent->child_count] = child;
	parent->child_count++;
}

// Core Parser functions ========================================================

MCNode_t *markcore_parse(const char *markdown, size_t len) {

	MCNode_t *root = create_node(ROOT_NODE, NULL, 0);

	if (node_stack) stack_free(node_stack);
	node_stack = stack_create(4);
	stack_push(node_stack, root);

	size_t current_pos = 0;
	while (current_pos < len) {
		size_t line_len = 0;
		while ((current_pos + line_len) < len && markdown[current_pos + line_len] != '\0' && markdown[current_pos + line_len] != '\n')
            line_len++;

		markcore_parse_line(markdown, current_pos, current_pos + line_len);

		current_pos += line_len;
		if (current_pos < len && markdown[current_pos] == '\n') current_pos++;
	}
	
	stack_free(node_stack);

	return root;
}

// Helper ======================================================

static const char *seek_next_char(const char *p, const char *limit, const char c) {
	while (p < limit && *p != '\0') {
		if (*p == c) {
			return p;
		}
		p++;
	}
	return NULL;
}

static size_t is_ordered_list_item(const char **p_ptr, const char *limit) {
	
	const char *p = *p_ptr;
	
    if (p >= limit || !isdigit((unsigned char)*p)) return 0;
    while (p < limit && isdigit((unsigned char)*p)) p++;

    if (p >= limit || (*p != '.' && *p != ')')) return 0;
    p++; // skip . or )

    if (p >= limit || *p != ' ') return 0;
    
    *p_ptr = p; // set read head after list entry point

    return 1;
}

// add text node to parent (call this right before adding a bold child node for example)
static void flush_text(const char *start, const char *end) {
	if (start >= end) return;
	size_t len = end - start;
	
	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *text_node = create_node(TEXT_NODE, start, len);
	add_child_node(top_node, text_node);
}

// Inline Methods ==============================================

static MCNode_t *markcore_parse_link(const char **p_ptr, const char *limit) {

	const char *p = *p_ptr;
	const char *start = p;
	
	const char *close_bracket = seek_next_char(p, limit, ']');
	if (!close_bracket) return NULL;
	
	p = close_bracket + 1;
	if (p >= limit || *p != '(') return NULL;
	const char *open_link = p;
	
	const char *close_link = seek_next_char(p, limit, ')');
	if (!close_link) return NULL;

	p = start + 1; // set read head to start of text label
	size_t text_len = close_bracket - p;

	// Text label is the content
	MCNode_t *link_node = create_node(LINK_NODE, p, text_len);

	// URL is the data
	p = close_bracket + 2; // set read head to start of url
	size_t url_len = close_link - open_link - 1;
    
	link_node->data = p;
	link_node->data_len = url_len;
	
	*p_ptr = close_link + 1; // set read head
	
	return link_node;
}

static MCNode_t *markcore_parse_inline_code(const char **p_ptr, const char *limit) {

	const char *p = *p_ptr;
	const char *start = p;
	
	p++;
	
	const char *close_tick = seek_next_char(p, limit, '`');
	if (!close_tick) return NULL;

	p = start + 1; // set read head to start of text label
	size_t text_len = close_tick - p;
    
	MCNode_t *inline_code_node = create_node(CODE_INLINE_NODE, p, text_len);
	
	*p_ptr = close_tick + 1; // set read head
	
	return inline_code_node;
}

static MCNode_t *markcore_parse_italics_bold(const char **p_ptr, const char *limit) {
	
	const char *p = *p_ptr;
	const char *start = p;
		
	int delimiter_count = 0;
	while (p < limit && *p == '*' && delimiter_count < 3) { delimiter_count++; p++; };
		
	const char *next_delimiter = seek_next_char(p, limit, '*');
	if (!next_delimiter) return NULL;

	for (;;) {
		if (delimiter_count == 1) {
			break;
		}
		if (delimiter_count == 2) {
			if ((next_delimiter + 1) < limit && *(next_delimiter + 1) == '*') {
				break;
			}
		} else if (delimiter_count == 3) {
			if ((next_delimiter + 2) < limit && *(next_delimiter + 1) == '*' && *(next_delimiter + 2) == '*') {
				break;
			}
		}
		next_delimiter = seek_next_char(next_delimiter + 1, limit, '*');
		if (!next_delimiter) return NULL;
	}
				
	MCNode_t *italics_bold_node = NULL;
	switch (delimiter_count) {
		case 1: italics_bold_node = create_node(ITALIC_NODE, NULL, 0); break;
		case 2: italics_bold_node = create_node(BOLD_NODE, NULL, 0); break;
		case 3: italics_bold_node = create_node(BOLD_ITALIC_NODE, NULL, 0); break;
	}
	
	if (!italics_bold_node) return NULL; // Should not happen given logic above
	
	p = start + delimiter_count;

	stack_push(node_stack, italics_bold_node);
	markcore_parse_inline_range(p, next_delimiter);
	(void)stack_pop(node_stack);
	
	*p_ptr = next_delimiter + delimiter_count;
	
	return italics_bold_node;
}

// recursive tree builder for inline parsing, cature and handle bold, italics, links, etc.
static void markcore_parse_inline_range(const char *start, const char *end) {
	
	const char *p = start;
	
	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *new_node;
		
	const char *last_text = start;
	const char *og_p;
	while (p < end) {
		og_p = p;
		switch (*p) {
		case '[': // links
			new_node = markcore_parse_link(&p, end);
			if (new_node) { 
				flush_text(last_text, og_p);
				last_text = p;
				add_child_node(top_node, new_node);
			} else {
				// Failed to parse link, treat as text
			}
 			break;
 		case '*':
			new_node = markcore_parse_italics_bold(&p, end);
 			if (new_node) {
				flush_text(last_text, og_p);
				last_text = p;
 				add_child_node(top_node, new_node);
			} else {
				// Failed to parse bold/italic, treat as text
			}
 			break;
 		case '`': // inline code
			new_node = markcore_parse_inline_code(&p, end);
 			if (new_node) {
				flush_text(last_text, og_p);
				last_text = p;
 				add_child_node(top_node, new_node);
			} else {
				// Failed to parse code, treat as text
			}
		default:
			break;
		}
		p++;
	}
	if (last_text < end) {
		flush_text(last_text, end); // flush remaining text
	}
}

// Full Lines ==========================================================

static MCNode_t *markcore_parse_image(const char *p, const char *limit) {

	const char *start = p;
	
	p++;
	if (p >= limit || *p != '[') return NULL;
	
	const char *close_bracket = seek_next_char(p, limit, ']');
	if (!close_bracket) return NULL;
	
	p = close_bracket + 1;
	if (p >= limit || *p != '(') return NULL;
	const char *open_link = p;
	
	const char *close_link = seek_next_char(p, limit, ')');
	if (!close_link) return NULL;

	p = start + 2; // set read head to start of text label
	size_t text_len = close_bracket - p;
		
	// Image alt text is content
	MCNode_t *image_node = create_node(IMAGE_NODE, p, text_len);

	// URL is data
	p = close_bracket + 2;
	size_t url_len = close_link - open_link - 1;
    
	image_node->data = p;
	image_node->data_len = url_len;
	
	return image_node;
}

static MCNode_t *markcore_parse_header(const char *p, const char *limit) {
	// heading, count number
	int header_count = 0;
	const char *orig_p = p;
	while (p < limit && *p == '#') { header_count++; p++; };
	
	// Content starts after #
	MCNode_t *header_node = create_node(HEADER_NODE, p, limit - p);
	header_node->header_level = header_count;
	return header_node;
}

static void escape_if_in_list(MCNode_t **top_node) {
	if ((*top_node)->type == UNORDERED_LIST_NODE || (*top_node)->type == ORDERED_LIST_NODE) {
		// skip multi line
		(void)stack_pop(node_stack);
		*top_node = stack_peek(node_stack);
	}
}

static void markcore_parse_line(const char *markdown, size_t start_idx, size_t end_idx) {
	// construct tree for start line
	const char *start = markdown + start_idx;
	const char *end = markdown + end_idx;
	const char *p = start;

	while (p < end && (*p == ' ' || *p == '\t')) p++; // trim leading whitespace
	if (p == end) return; // skip empty lines (or lines with just whitespace)
	
	MCNode_t *top_node = stack_peek(node_stack);
	if (!top_node) {
		fprintf(stderr, "Error, stack is empty\n");
		return;
	}
	
	MCNode_t *temp_node; // for header / image creation
	
	// Skip formatting if in code block
	if (top_node->type == CODE_BLOCK_NODE && (end - p < 3 || strncmp(p, "```", 3) != 0)) {
		flush_text(start, end);
		return;
	}
	
	switch (*p) {
		case '#':
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_header(p, end);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '!': // check for image
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_image(p, end);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '*': // check for bullet first, then inline
			if ((p + 1) < end && *(p + 1) == ' ') {
				// bullet
				if (top_node->type != UNORDERED_LIST_NODE) {
					MCNode_t *list_node = create_node(UNORDERED_LIST_NODE, NULL, 0);
					add_child_node(top_node, list_node);
					stack_push(node_stack, list_node);
					top_node = list_node;
				}
				p++; // advance over bullet point
			}
			break;
		case '`': // code blocks not inline
		
			if ((end - p) >= 3 && strncmp(p, "```", 3) == 0) {
				// code block!
				if (top_node->type != CODE_BLOCK_NODE) {
					MCNode_t *code_block_node = create_node(CODE_BLOCK_NODE, NULL, 0);
					add_child_node(top_node, code_block_node);
					stack_push(node_stack, code_block_node);
					top_node = code_block_node;
					return; // start next line
				} else {
					(void)stack_pop(node_stack);
					top_node = stack_peek(node_stack);
					return; // start next line
				}
			}
			
			break;
		default:
		
			if (is_ordered_list_item(&p, end)) {
				// ordered list
				if (top_node->type != ORDERED_LIST_NODE) {
					MCNode_t *list_node = create_node(ORDERED_LIST_NODE, NULL, 0);
					add_child_node(top_node, list_node);
					stack_push(node_stack, list_node);
					top_node = list_node;
				}	
			} else {
				escape_if_in_list(&top_node);
			} 
	}
	
	MCNode_t *line_node = create_node(LINE_NODE, NULL, 0);
	stack_push(node_stack, line_node);
	add_child_node(top_node, line_node);
	markcore_parse_inline_range(p, end);
	(void)stack_pop(node_stack);	
}

void markcore_free_syntax_tree(MCNode_t *node) {
	// 	DFS, free buffers and free nodes
	if (!node) return;
	
	// node->content and node->data are now views, DO NOT FREE them
	
	for (int i = 0; i < node->child_count; i++) {
		MCNode_t *child = node->children[i];
		markcore_free_syntax_tree(child);
	}
	
	if (node->children) free(node->children);
	
	free(node);
	
}

// DEBUG ===========================================

void markcore_print_tree(MCNode_t *node, int depth) {
	if (!node) return;
	// DFS
	int i;
	for (i = 0; i < depth; i++) {
		printf("\t");
	}
	
	if (depth > 0) {
		printf("└── ");
	}
	
	switch (node->type) {
		case LINK_NODE:
			printf("Link – %.*s (%.*s)\n", (int)node->content_len, node->content, (int)node->data_len, node->data);
			break;
		case HEADER_NODE:
			printf("Header %i\n", node->header_level);
			break;
		case TEXT_NODE:
			printf("Text – %.*s\n", (int)node->content_len, node->content);
			break;
		case IMAGE_NODE:
			printf("Image – %.*s\n", (int)node->data_len, node->data);
			break;
		case CODE_INLINE_NODE:
			printf("Inline code – %.*s\n", (int)node->content_len, node->content);
			break;
		default:
			printf("%s\n", type_labels[node->type]);
	}
    
    for (i = 0; i < node->child_count; i++) {
    	if (node->children[i]) {
			markcore_print_tree(node->children[i], depth + 1);
    	}
	}
}
