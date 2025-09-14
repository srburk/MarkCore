
#include "parser.h"
#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define LINE_BUFFER_SIZE 1024
#define INITIAL_CHILD_CAPACITY 1

static Stack_t *node_stack;

// Forward declaration ======================================================

static void markcore_parse_line(char *markdown, size_t len);

static MCNode_t *markcore_parse_image(char *p);

static void markcore_parse_inline_range(char *start, char *end);

static MCNode_t *markcore_parse_link(char **p_ptr);
static MCNode_t *markcore_parse_italics_bold(char **p_ptr);

static void flush_text(char *start, char *end);

static void debug_print_range(const char *start, const char *end, const char *label);

// Tree functions

static MCNode_t *create_node(MCNodeType_e type, const char *content) {
	MCNode_t *node = malloc(sizeof(MCNode_t));
    if (!node) return NULL;
    node->type = type;
    node->content = content ? strdup(content) : NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
	node->data = NULL;
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

// static void free_tree(MCNode_t *root) {
// 	// clean up entire tree
// }

// Core Parser functions ========================================================

MCNode_t *markcore_parse(char *markdown, size_t len) {

	MCNode_t *root = create_node(ROOT_NODE, NULL);

	char *p = markdown;
	char line[LINE_BUFFER_SIZE];
	
// 	MCParserContext_t context = {};
// 	context.stack = stack_create(4);
	if (node_stack) stack_free(node_stack);
	node_stack = stack_create(4);
	stack_push(node_stack, root);

	while (*p && ((size_t)(p - markdown) < len)) {
		size_t line_len = 0;
		while ((size_t)(p + line_len - markdown) < len && p[line_len] && p[line_len] != '\n')
            line_len++;
        if (line_len >= LINE_BUFFER_SIZE) {
        	line_len = LINE_BUFFER_SIZE - 1;
        	fprintf(stderr, "Line is too long: %s\n", p);
		}
		strncpy(line, p, line_len);
		line[line_len] = '\0';
		
		markcore_parse_line(line, line_len);

		p += line_len;
		
		if (*p == '\n') p++;
	}
	
	stack_free(node_stack);

	return root;
}

// Helper ======================================================

static char *seek_next_char(char *p, const char c) {
	while (*p != '\0') {
		if (*p == c) {
			return p;
		}
		p++;
	}
	return NULL;
}

static size_t is_ordered_list_item(const char **p_ptr) {
	
	char *p = *p_ptr;
	
    if (*p == '\0' || !isdigit((unsigned char)*p)) return 0;
    while (*p != '\0' && isdigit((unsigned char)*p)) p++;

    if (*p == '\0' || (*p != '.' && *p != ')')) return 0;
    p++; // skip . or )

    if (*p == '\0' || *p != ' ') return 0;
    
    *p_ptr = p; // set read head after list entry point

    return 1;
}

// add text node to parent (call this right before adding a bold child node for example)
static void flush_text(char *start, char *end) {
	if (start == end || start > end) return;
	size_t len = end - start;
	char *text_buffer = malloc(len + 1);
	strncpy(text_buffer, start, len);
	text_buffer[len] = '\0';
	
	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *text_node = create_node(TEXT_NODE, text_buffer);
	add_child_node(top_node, text_node);
}

// Inline Methods ==============================================

static MCNode_t *markcore_parse_link(char **p_ptr) {

	char *p = *p_ptr;
	char *start = p;
	
	char *close_bracket = seek_next_char(p, ']');
	if (!close_bracket) return NULL;
	
	p = close_bracket + 1;
	if (*p != '(') return NULL;
	char *open_link = p;
	
	char *close_link = seek_next_char(p, ')');
	if (!close_link) return NULL;

	p = start + 1; // set read head to start of text label
	size_t text_len = close_bracket - p;
	char *text = malloc(text_len + 1);
	strncpy(text, p, text_len);
	text[text_len] = '\0';
		
	p = close_bracket + 2; // set read head to start of url
	size_t url_len = close_link - open_link - 1;
    char *url = malloc(url_len + 1);
    memcpy(url, p, url_len);
    url[url_len] = '\0';
    
	MCNode_t *link_node = create_node(LINK_NODE, text);
	link_node->data = url;
	
	*p_ptr = close_link + 1; // set read head
	return link_node;
}

static MCNode_t *markcore_parse_inline_code(char **p_ptr) {

	char *p = *p_ptr;
	char *start = p;
	
	p++;
	
	char *close_tick = seek_next_char(p, '`');
	if (!close_tick) return NULL;

	p = start + 1; // set read head to start of text labe
	size_t text_len = close_tick - p;
	char *text = malloc(text_len + 1);
	strncpy(text, p, text_len);
	text[text_len] = '\0';
    
	MCNode_t *inline_code_node = create_node(CODE_INLINE_NODE, text);
	
	*p_ptr = close_tick + 1; // set read head
	return inline_code_node;
}

static MCNode_t *markcore_parse_italics_bold(char **p_ptr) {
	
	char *p = *p_ptr;
	char *start = p;
		
	int delimiter_count = 0;
	while (*p != '\0' && *p == '*' && delimiter_count < 3) { delimiter_count++; p++; };
		
	char *next_delimiter = seek_next_char(p, '*');
	if (!next_delimiter) return NULL;

	for (;;) {
		if (delimiter_count == 1) {
			break;
		}
		if (delimiter_count == 2) {
			if (*(next_delimiter + 1) == '*') {
				break;
			}
		} else if (delimiter_count == 3) {
			if (*(next_delimiter + 1) == '*' && *(next_delimiter + 2) == '*') {
				break;
			}
		}
		next_delimiter = seek_next_char(next_delimiter + 1, '*');
		if (!next_delimiter) return NULL;
	}
				
	MCNode_t *italics_bold_node;
	switch (delimiter_count) {
		case 1: italics_bold_node = create_node(ITALIC_NODE, NULL); break;
		case 2: italics_bold_node = create_node(BOLD_NODE, NULL); break;
		case 3: italics_bold_node = create_node(BOLD_ITALIC_NODE, NULL); break;
	}
	
	
	p = start + delimiter_count;

	stack_push(node_stack, italics_bold_node);
	markcore_parse_inline_range(p, next_delimiter);
	(void)stack_pop(node_stack);
	
	*p_ptr = next_delimiter + delimiter_count;
	
	return italics_bold_node;
}

// recursive tree builder for inline parsing, cature and handle bold, italics, links, etc.
static void markcore_parse_inline_range(char *start, char *end) {	
	
	char *p = start;
	
	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *new_node;
		
	char *last_text = start;
	char *og_p;
	while (p < end) {
		og_p = p;
		switch (*p) {
		case '[': // links
			new_node = markcore_parse_link(&p);
			if (new_node) { 
				flush_text(last_text, og_p);
				last_text = p;
				add_child_node(top_node, new_node);
			}
 			break;
 		case '*':
 			new_node = markcore_parse_italics_bold(&p);
 			if (new_node) {
				flush_text(last_text, og_p);
				last_text = p;
 				add_child_node(top_node, new_node);
 			}
 			break;
 		case '`': // inline code
 			new_node = markcore_parse_inline_code(&p);
 			if (new_node) {
				flush_text(last_text, og_p);
				last_text = p;
 				add_child_node(top_node, new_node);
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

static MCNode_t *markcore_parse_image(char *p) {

	char *start = p;
	
	p++;
	if (*p != '[') return NULL;
	
	char *close_bracket = seek_next_char(p, ']');
	if (!close_bracket) return NULL;
	
	p = close_bracket + 1;
	if (*p != '(') return NULL;
	char *open_link = p;
	
	char *close_link = seek_next_char(p, ')');
	if (!close_link) return NULL;

	p = start + 2; // set read head to start of text label
	size_t text_len = close_bracket - p;
	char *text = malloc(text_len + 1);
	strncpy(text, p, text_len);
	text[text_len] = '\0';
		
	p = close_bracket + 2; // set read head to start of url
	size_t url_len = close_link - open_link - 1;
    char *url = malloc(url_len + 1);
    memcpy(url, p, url_len);
    url[url_len] = '\0';
    
	MCNode_t *link_node = create_node(IMAGE_NODE, text);
	link_node->data = url;
	
	return link_node;
}

static MCNode_t *markcore_parse_header(char *p) {
	// heading, count number
	int header_count = 0;
	while (*p != '\0' && *p == '#') { header_count++; p++; };
	
	// convert root to header
	char *content = strdup(p);
	if (!content) {
		fprintf(stderr, "Failed to copy content: %s\n", p);
		return NULL;
	}
	
	MCNode_t *header_node = create_node(HEADER_NODE, NULL);
	
	header_node->content = content;
	header_node->header_level = header_count;
	return header_node;
}

static void markcore_parse_line(char *start, size_t len) {
	// construct tree for start line
	char *p = start;	
	while (*p == ' ' || *p == '\t') p++; // trim leading whitespace
	if (*p == '\n' || *p == '\0') return; // skip empty lines
	
	MCNode_t *top_node = stack_peek(node_stack);
	if (!top_node) {
		fprintf(stderr, "Error, stack is empty\n");
		return;
	}
	
	MCNode_t *temp_node; // for header / image creation
	
	// Skip formatting if in code block
	if (top_node->type == CODE_BLOCK_NODE && strncmp(p, "```", 3) != 0) {
		flush_text(start, start + len);
		return;
	}
	
	switch (*p) {
		case '#':
			temp_node = markcore_parse_header(p);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '!': // check for image
			temp_node = markcore_parse_image(p);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '*': // check for bullet first, then inline
			if (*(p + 1) == ' ') {
				// bullet
				if (top_node->type != UNORDERED_LIST_NODE) {
					MCNode_t *list_node = create_node(UNORDERED_LIST_NODE, NULL);
					add_child_node(top_node, list_node);
					stack_push(node_stack, list_node);
					top_node = list_node;
				}
				p++; // advance over bullet point
			}
			break;
		case '`': // code blocks not inline
		
			if (strncmp(p, "```", 3) == 0) {
				// code block!
				if (top_node->type != CODE_BLOCK_NODE) {
					MCNode_t *code_block_node = create_node(CODE_BLOCK_NODE, NULL);
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
		
			if (is_ordered_list_item(&p)) {
				// ordered list
				if (top_node->type != ORDERED_LIST_NODE) {
					MCNode_t *list_node = create_node(ORDERED_LIST_NODE, NULL);
					add_child_node(top_node, list_node);
					stack_push(node_stack, list_node);
					top_node = list_node;
				}	
			} else if (top_node->type == UNORDERED_LIST_NODE || top_node->type == ORDERED_LIST_NODE) {
				// skip multi line
				(void)stack_pop(node_stack);
				top_node = stack_peek(node_stack);
			}
	}
	
	MCNode_t *line_node = create_node(LINE_NODE, NULL);
	stack_push(node_stack, line_node);
	add_child_node(top_node, line_node);
	markcore_parse_inline_range(p, start+len);
	(void)stack_pop(node_stack);	
}

void markcore_free_syntax_tree(MCNode_t *node) {
	// 	DFS, free buffers and free nodes
	if (!node) return;
	for (int i = 0; i < node->child_count; i++) {
		MCNode_t *child = node->children[i];
    	if (child) {
			markcore_free_syntax_tree(child);
			if (child->content) free(child->content);
			if (child->data) free(child->data);
			free(child);
    	}
	}
}

// DEBUG ===========================================

static void debug_print_range(const char *start, const char *end, const char *label) {
    if (!start || !end || start >= end) {
        printf("[DEBUG] %s: <empty or invalid range> (%p to %p)\n", label ? label : "range", start, end);
        return;
    }
    size_t len = end - start;
    char *buffer = malloc(len + 1);
    if (!buffer) {
        fprintf(stderr, "[DEBUG] %s: failed to allocate buffer\n", label ? label : "range");
        return;
    }
    memcpy(buffer, start, len);
    buffer[len] = '\0';
    printf("[DEBUG] %s: \"%s\"\n", label ? label : "range", buffer);
    free(buffer);
}

void print_node(void *n) {
    MCNode_t *node = (MCNode_t*)n;
    printf("{type=%s}", type_labels[node->type]);
}

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
			printf("Link – %s (%s)\n", node->content, node->data);
			break;
		case HEADER_NODE:
			printf("Header %i\n", node->header_level);
			break;
		case TEXT_NODE:
			printf("Text – %s\n", node->content);
			break;
		case IMAGE_NODE:
			printf("Image – %s\n", node->data);
			break;
		case CODE_INLINE_NODE:
			printf("Inline code – %s\n", node->content);
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
