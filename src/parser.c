
#include "parser.h"

#include <string.h>
#include <stdio.h>

#define LINE_BUFFER_SIZE 1024

#define INITIAL_CHILD_CAPACITY 1

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
		add_child_node(root, line_node);
		
		p += line_len;
		
		if (*p == '\n') p++;
	}

	return root;
}

MarkCoreNode_t *markcore_parse_line(char *markdown, size_t len) {
	// construct tree for markdown line
	
	MarkCoreNode_t *root = create_node(ROOT_NODE, NULL);
	MarkCoreNode_t *current = root;
	
	char *p = markdown;
	
	while (*p == ' ' || *p == '\t') p++; // trim leading whitespace
		
	int header_count;
	switch (*p) {
		case '#':
			// heading, count number
			header_count = 0;
			while ((size_t)(p - markdown) < len && *p == '#') { header_count++; p++; };
			MarkCoreNode_t *header_node = create_node(HEADER_NODE, p);
			header_node->header_level = header_count;
			add_child_node(current, header_node);
			return root; // header have no inline rendering
			break;
		default:
			// probably just a regular, check overall state to determine if in block quote or other
			break;
	}
	
	return root;
}

// DEBUG ===========================================

static char *type_labels[NODE_TYPE_COUNT] = {
	[ROOT_NODE] = "Root",
	[HEADER_NODE] = "Header"
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
	
	if (node->type == HEADER_NODE) {
		printf("Header %i – %s\n", node->header_level, node->content);
	} else {
		printf("%s\n", type_labels[node->type]);	
	}
    
    for (i = 0; i < node->child_count; i++) {
    	if (node->children[i]) {
			markcore_print_tree(node->children[i], depth + 1);
    	}
	}
}
