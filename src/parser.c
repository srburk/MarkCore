
#include "parser.h"
#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define INITIAL_CHILD_CAPACITY 4

static Stack_t *node_stack = NULL;

// Forward declaration ======================================================

static void markcore_parse_line(char *markdown, size_t len);

static MCNode_t *markcore_parse_image(char *p);

static void markcore_parse_inline_range(char *start, char *end);

static MCNode_t *markcore_parse_link(char **p_ptr);
static MCNode_t *markcore_parse_italics_bold(char **p_ptr);

static void flush_text(char *start, char *end);

// Tree functions

static MCNode_t *create_node(MCNodeType_e type, const char *content) {
	MCNode_t *node = calloc(1, sizeof(MCNode_t));
	if (!node) return NULL;
	node->type = type;
	if (content) {
		node->content = strdup(content);
		if (!node->content) {
			free(node);
			return NULL;
		}
	}
	return node;
}

static void add_child_node(MCNode_t *parent, MCNode_t *child) {
	if (!parent || !child) return;

	if (!parent->children) {
		parent->children = malloc(sizeof(MCNode_t *) * INITIAL_CHILD_CAPACITY);
		if (!parent->children) {
			fprintf(stderr, "Failed to allocate children\n");
			return;
		}
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

	if (!markdown) return NULL;

	MCNode_t *root = create_node(ROOT_NODE, NULL);
	if (!root) return NULL;

	if (node_stack) {
		stack_free(node_stack);
		node_stack = NULL;
	}
	node_stack = stack_create(4);
	if (!node_stack) {
		markcore_free_syntax_tree(root);
		return NULL;
	}
	stack_push(node_stack, root);

	const char *end = markdown + len;
	const char *p = markdown;

	while (p < end) {
		const char *line_end = p;
		while (line_end < end && *line_end != '\n') {
			line_end++;
		}
		size_t line_len = (size_t)(line_end - p);

		char *line = malloc(line_len + 1);
		if (!line) {
			fprintf(stderr, "Failed to allocate line buffer\n");
			break;
		}
		memcpy(line, p, line_len);
		line[line_len] = '\0';

		markcore_parse_line(line, line_len);
		free(line);

		p = line_end;
		if (p < end && *p == '\n') p++;
	}

	stack_free(node_stack);
	node_stack = NULL;

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

static size_t is_ordered_list_item(char **p_ptr) {
	
	char *p = *p_ptr;
	
    if (*p == '\0' || !isdigit((unsigned char)*p)) return 0;
    while (*p != '\0' && isdigit((unsigned char)*p)) p++;

    if (*p == '\0' || (*p != '.' && *p != ')')) return 0;
    p++; // skip . or )

    if (*p == '\0' || *p != ' ') return 0;
    p++; // skip the required space after . or )

    *p_ptr = p; // set read head after list marker

    return 1;
}

// add text node to parent (call this right before adding a bold child node for example)
static void flush_text(char *start, char *end) {
	if (!start || !end || start >= end) return;
	size_t len = (size_t)(end - start);
	char *text_buffer = malloc(len + 1);
	if (!text_buffer) return;
	memcpy(text_buffer, start, len);
	text_buffer[len] = '\0';

	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *text_node = create_node(TEXT_NODE, text_buffer);
	add_child_node(top_node, text_node);

	free(text_buffer); // (it's strduped in create_node)
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
	size_t text_len = (size_t)(close_bracket - p);
	char *text = malloc(text_len + 1);
	if (!text) return NULL;
	memcpy(text, p, text_len);
	text[text_len] = '\0';

	p = close_bracket + 2; // set read head to start of url
	size_t url_len = (size_t)(close_link - open_link - 1);
	char *url = malloc(url_len + 1);
	if (!url) {
		free(text);
		return NULL;
	}
	memcpy(url, p, url_len);
	url[url_len] = '\0';

	MCNode_t *link_node = create_node(LINK_NODE, text);
	free(text);
	if (!link_node) {
		free(url);
		return NULL;
	}
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

	p = start + 1; // set read head to start of text
	size_t text_len = (size_t)(close_tick - p);
	char *text = malloc(text_len + 1);
	if (!text) return NULL;
	memcpy(text, p, text_len);
	text[text_len] = '\0';

	MCNode_t *inline_code_node = create_node(CODE_INLINE_NODE, text);
	free(text);
	if (!inline_code_node) return NULL;

	*p_ptr = close_tick + 1; // set read head

	return inline_code_node;
}

static int count_stars(const char *p) {
	int n = 0;
	while (p[n] == '*' && n < 3) n++;
	return n;
}

static MCNode_t *markcore_parse_italics_bold(char **p_ptr) {

	char *p = *p_ptr;
	char *start = p;

	int delimiter_count = count_stars(p);
	if (delimiter_count == 0) return NULL;
	p += delimiter_count;

	char *next_delimiter = seek_next_char(p, '*');
	if (!next_delimiter) return NULL;

	for (;;) {
		int close_count = count_stars(next_delimiter);
		if (close_count >= delimiter_count) {
			break;
		}
		next_delimiter = seek_next_char(next_delimiter + close_count, '*');
		if (!next_delimiter) return NULL;
	}

	MCNode_t *italics_bold_node = NULL;
	switch (delimiter_count) {
		case 1: italics_bold_node = create_node(ITALIC_NODE, NULL); break;
		case 2: italics_bold_node = create_node(BOLD_NODE, NULL); break;
		case 3: italics_bold_node = create_node(BOLD_ITALIC_NODE, NULL); break;
	}
	if (!italics_bold_node) return NULL;

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
	if (!top_node) return;

	char *last_text = start;
	while (p < end) {
		char *og_p = p;
		MCNode_t *new_node = NULL;
		switch (*p) {
		case '[':
			new_node = markcore_parse_link(&p);
			break;
		case '*':
			new_node = markcore_parse_italics_bold(&p);
			break;
		case '`':
			new_node = markcore_parse_inline_code(&p);
			break;
		default:
			break;
		}
		if (new_node) {
			flush_text(last_text, og_p);
			last_text = p;
			add_child_node(top_node, new_node);
			continue; /* helpers already advanced p past the construct */
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
	size_t text_len = (size_t)(close_bracket - p);
	char *text = malloc(text_len + 1);
	if (!text) return NULL;
	memcpy(text, p, text_len);
	text[text_len] = '\0';

	p = close_bracket + 2; // set read head to start of url
	size_t url_len = (size_t)(close_link - open_link - 1);
	char *url = malloc(url_len + 1);
	if (!url) {
		free(text);
		return NULL;
	}
	memcpy(url, p, url_len);
	url[url_len] = '\0';

	MCNode_t *image_node = create_node(IMAGE_NODE, text);
	free(text);
	if (!image_node) {
		free(url);
		return NULL;
	}
	image_node->data = url;

	return image_node;
}

static MCNode_t *markcore_parse_header(char *p) {
	int header_count = 0;
	while (*p != '\0' && *p == '#') { header_count++; p++; }

	// CommonMark ATX headings are 1–6 hashes
	if (header_count < 1 || header_count > 6) return NULL;

	if (*p == ' ') p++; // drop the conventional space after hashes

	MCNode_t *header_node = create_node(HEADER_NODE, NULL);
	if (!header_node) return NULL;

	header_node->content = strdup(p);
	if (!header_node->content) {
		free(header_node);
		return NULL;
	}
	header_node->header_level = header_count;
	return header_node;
}

static void escape_if_in_list(MCNode_t **top_node) {
	if (!top_node || !*top_node) return;
	if ((*top_node)->type == UNORDERED_LIST_NODE || (*top_node)->type == ORDERED_LIST_NODE) {
		(void)stack_pop(node_stack);
		*top_node = stack_peek(node_stack);
	}
}

static void ensure_list_context(MCNode_t **top_node, MCNodeType_e list_type) {
	if (!top_node || !*top_node) return;
	if ((*top_node)->type == list_type) return;

	escape_if_in_list(top_node);
	if (!*top_node) return;

	MCNode_t *list_node = create_node(list_type, NULL);
	if (!list_node) return;
	add_child_node(*top_node, list_node);
	stack_push(node_stack, list_node);
	*top_node = list_node;
}

static void markcore_parse_line(char *start, size_t len) {
	char *p = start;
	while (*p == ' ' || *p == '\t') p++; // trim leading whitespace

	MCNode_t *top_node = stack_peek(node_stack);
	if (!top_node) {
		fprintf(stderr, "Error, stack is empty\n");
		return;
	}

	if (*p == '\n' || *p == '\0') {
		// Preserve blank lines inside fenced code; skip them elsewhere
		if (top_node->type == CODE_BLOCK_NODE) {
			MCNode_t *text_node = create_node(TEXT_NODE, "");
			add_child_node(top_node, text_node);
		}
		return;
	}

	MCNode_t *temp_node;

	if (top_node->type == CODE_BLOCK_NODE && strncmp(p, "```", 3) != 0) {
		flush_text(start, start + len);
		return;
	}

	switch (*p) {
		case '#':
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_header(p);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '!':
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_image(p);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '*':
			if (*(p + 1) == ' ') {
				ensure_list_context(&top_node, UNORDERED_LIST_NODE);
				p += 2; // skip "* "
			}
			break;
		case '`':
			if (strncmp(p, "```", 3) == 0) {
				if (top_node->type != CODE_BLOCK_NODE) {
					escape_if_in_list(&top_node);
					MCNode_t *code_block_node = create_node(CODE_BLOCK_NODE, NULL);
					add_child_node(top_node, code_block_node);
					stack_push(node_stack, code_block_node);
				} else {
					(void)stack_pop(node_stack);
				}
				return;
			}
			break;
		default:
			if (is_ordered_list_item(&p)) {
				ensure_list_context(&top_node, ORDERED_LIST_NODE);
			} else {
				escape_if_in_list(&top_node);
			}
	}

	MCNode_t *line_node = create_node(LINE_NODE, NULL);
	if (!line_node) return;
	stack_push(node_stack, line_node);
	add_child_node(top_node, line_node);
	markcore_parse_inline_range(p, start + len);
	(void)stack_pop(node_stack);
}

void markcore_free_syntax_tree(MCNode_t *node) {
	// 	DFS, free buffers and free nodes
	if (!node) return;
	
	if (node->content) free(node->content);
	if (node->data) free(node->data);
	
	for (int i = 0; i < node->child_count; i++) {
		MCNode_t *child = node->children[i];
		markcore_free_syntax_tree(child);
	}
	
	if (node->children) free(node->children);
	
	free(node);

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
