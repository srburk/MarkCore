
#include "parser.h"

#include <string.h>
#include <stdio.h>

#define LINE_BUFFER_SIZE 1024

#define INITIAL_CHILD_CAPACITY 1

// Forward declaration

static MarkCoreNode_t *markcore_parse_image(char *p);

static void markcore_parse_inline_range(MarkCoreNode_t *parent_node, char *start, char *end);

static MarkCoreNode_t *markcore_parse_link(char **p_ptr);
static MarkCoreNode_t *markcore_parse_italics_bold(char **p_ptr);

static void flush_text(MarkCoreNode_t *parent_node, char *start, char *end);

static void debug_print_range(const char *start, const char *end, const char *label);

// Tree functions

static MarkCoreNode_t *create_node(MarkCoreNodeType_t type, const char *content) {
	MarkCoreNode_t *node = malloc(sizeof(MarkCoreNode_t));
    if (!node) return NULL;

    node->type = type;
    node->content = content ? strdup(content) : NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;

    return node;
}

static void add_child_node(MarkCoreNode_t *parent, MarkCoreNode_t *child) {
	if (!parent || !child) return;
	
	if (!parent->children) {
		// initialize
		parent->children = malloc(sizeof(MarkCoreNode_t *) * INITIAL_CHILD_CAPACITY);
		parent->child_capacity = INITIAL_CHILD_CAPACITY;
	}
	
	if (parent->child_count + 1 > parent->child_capacity) {
		size_t new_capacity = parent->child_capacity * 2;
		MarkCoreNode_t **new_children = realloc(parent->children, sizeof(MarkCoreNode_t *) * new_capacity);
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

// static void free_tree(MarkCoreNode_t *root) {
// 	// clean up entire tree
// }

// Core Parser functions

MarkCoreNode_t *markcore_parse(char *markdown, size_t len) {

	MarkCoreNode_t *root = create_node(ROOT_NODE, NULL);

	char *p = markdown;
	char line[LINE_BUFFER_SIZE];

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
		
		MarkCoreNode_t *line_node = markcore_parse_line(line, line_len);
		if (line_node) {
				add_child_node(root, line_node);
		}

		p += line_len;
		
		if (*p == '\n') p++;
	}

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

// add text node to parent (call this right before adding a bold child node for example)
static void flush_text(MarkCoreNode_t *parent_node, char *start, char *end) {
	if (start == end || start > end) return;
	size_t len = end - start;
	char *text_buffer = malloc(len + 1);
	strncpy(text_buffer, start, len);
	text_buffer[len] = '\0';
			
	MarkCoreNode_t *text_node = create_node(TEXT_NODE, text_buffer);
	add_child_node(parent_node, text_node);
}

// Inline Methods ==============================================

static MarkCoreNode_t *markcore_parse_link(char **p_ptr) {

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
	char *text = malloc(text_len);
	strncpy(text, p, text_len);
	text[text_len] = '\0';
		
	p = close_bracket + 2; // set read head to start of url
	size_t url_len = close_link - open_link - 1;
    char *url = malloc(url_len + 1);
    memcpy(url, p, url_len);
    url[url_len] = '\0';
    
	MarkCoreNode_t *link_node = create_node(LINK_NODE, text);
	link_node->data = url;
	
	*p_ptr = close_link + 1; // set read head
	return link_node;
}

static MarkCoreNode_t *markcore_parse_italics_bold(char **p_ptr) {
	
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
				
	MarkCoreNode_t *italics_bold_node;
	switch (delimiter_count) {
		case 1: italics_bold_node = create_node(ITALIC_NODE, NULL); break;
		case 2: italics_bold_node = create_node(BOLD_NODE, NULL); break;
		case 3: italics_bold_node = create_node(BOLD_ITALIC_NODE, NULL); break;
	}
	
	p = start + delimiter_count;
	
	markcore_parse_inline_range(italics_bold_node, p, next_delimiter);
	
	*p_ptr = next_delimiter + delimiter_count;
	
	return italics_bold_node;
}

// recursive tree builder for inline parsing, cature and handle bold, italics, links, etc.
static void markcore_parse_inline_range(MarkCoreNode_t *parent_node, char *start, char *end) {	
	
	char *p = start;
	
	MarkCoreNode_t *new_node;
	
	char *last_text = start;
	char *og_p;
	while (p < end) {
		og_p = p;
		switch (*p) {
		case '[': // links
			new_node = markcore_parse_link(&p);
			if (new_node) { 
				flush_text(parent_node, last_text, og_p);
				last_text = p;
				add_child_node(parent_node, new_node);
			}
 			break;
 		case '*':
 			new_node = markcore_parse_italics_bold(&p);
 			if (new_node) {
				flush_text(parent_node, last_text, og_p);
				last_text = p;
 				add_child_node(parent_node, new_node);
 			}
 			break;
		default:
			break;
		}
		p++;
	}
	if (last_text < end) {
		flush_text(parent_node, last_text, end); // flush remaining text
	}
}

// Full Lines ==========================================================

static MarkCoreNode_t *markcore_parse_image(char *p) {

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
	char *text = malloc(text_len);
	strncpy(text, p, text_len);
	text[text_len] = '\0';
		
	p = close_bracket + 2; // set read head to start of url
	size_t url_len = close_link - open_link - 1;
    char *url = malloc(url_len + 1);
    memcpy(url, p, url_len);
    url[url_len] = '\0';
    
	MarkCoreNode_t *link_node = create_node(IMAGE_NODE, text);
	link_node->data = url;
	
	return link_node;
}

MarkCoreNode_t *markcore_parse_line(char *start, size_t len) {
	// construct tree for start line
	char *p = start;	
	while (*p == ' ' || *p == '\t') p++; // trim leading whitespace
	if (*p == '\n' || *p == '\0') return NULL; // skip empty lines
	
	MarkCoreNode_t *root = create_node(LINE_NODE, NULL);
	
	int header_count;
	char *content;
	
	switch (*p) {
		case '#':
			// heading, count number
			header_count = 0;
			while ((size_t)(p - start) < len && *p == '#') { header_count++; p++; };
			
			// convert root to header
			root->type = HEADER_NODE;
			content = strdup(p);
			if (!content) {
				fprintf(stderr, "Failed to copy content: %s\n", p);
			}
			root->content = content;
			root->header_level = header_count;
			return root; // header have no inline rendering
			break;
		case '!': // check for image
			markcore_parse_image(p);
			break;
		case '*': // check for bullet first, then inline
			if (*p + 1 == ' ') {
				// bullet
				markcore_parse_inline_range(root, p+1, start+len);
			} else {
				markcore_parse_inline_range(root, p, start+len);
			}
			break;
		default:
			// probably just a regular, check overall state to determine if in block quote or other
			markcore_parse_inline_range(root, p, start+len);
			break;
	}
	
	return root;
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

static char *type_labels[NODE_TYPE_COUNT] = {
	[ROOT_NODE] = "Root",
	[HEADER_NODE] = "Header",
	[LINK_NODE] = "Link",
	[IMAGE_NODE] = "Image",
	[LINE_NODE] = "Line",
	[BOLD_NODE] = "Bold",
	[ITALIC_NODE] = "Italic",
	[BOLD_ITALIC_NODE] = "Bold AND Italic",
	[TEXT_NODE] = "Text",
};

void markcore_print_tree(MarkCoreNode_t *node, int depth) {
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
		default:
			printf("%s\n", type_labels[node->type]);
	}
    
    for (i = 0; i < node->child_count; i++) {
    	if (node->children[i]) {
			markcore_print_tree(node->children[i], depth + 1);
    	}
	}
}
