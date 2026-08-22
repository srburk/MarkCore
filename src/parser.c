#include "parser.h"
#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define INITIAL_CHILD_CAPACITY 4

static Stack_t *node_stack = NULL;

static void markcore_parse_line(const char *start, const char *end);
static MCNode_t *markcore_parse_image(const char *p, const char *end);
static void markcore_parse_inline_range(const char *start, const char *end);
static MCNode_t *markcore_parse_link(const char **p_ptr, const char *end);
static MCNode_t *markcore_parse_italics_bold(const char **p_ptr, const char *end);
static MCNode_t *markcore_parse_inline_code(const char **p_ptr, const char *end);
static void flush_text(const char *start, const char *end);

static MCNode_t *create_node(MCNodeType_e type)
{
	MCNode_t *node = calloc(1, sizeof(MCNode_t));
	if (!node) return NULL;
	node->type = type;
	return node;
}

static void add_child_node(MCNode_t *parent, MCNode_t *child)
{
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
		size_t new_capacity = (size_t)parent->child_capacity * 2;
		MCNode_t **new_children = realloc(parent->children, sizeof(MCNode_t *) * new_capacity);
		if (!new_children) {
			fprintf(stderr, "Failed to realloc children\n");
			return;
		}
		parent->children = new_children;
		parent->child_capacity = (int)new_capacity;
	}
	parent->children[parent->child_count] = child;
	parent->child_count++;
}

MCNode_t *markcore_parse(const char *markdown, size_t len)
{
	if (!markdown) return NULL;

	MCNode_t *root = create_node(ROOT_NODE);
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
		markcore_parse_line(p, line_end);
		p = line_end;
		if (p < end && *p == '\n') p++;
	}

	stack_free(node_stack);
	node_stack = NULL;

	return root;
}

static const char *seek_next_char(const char *p, const char *end, char c)
{
	while (p < end) {
		if (*p == c) return p;
		p++;
	}
	return NULL;
}

static int has_prefix(const char *p, const char *end, const char *lit, size_t n)
{
	return (size_t)(end - p) >= n && memcmp(p, lit, n) == 0;
}

static int is_ordered_list_item(const char **p_ptr, const char *end)
{
	const char *p = *p_ptr;

	if (p >= end || !isdigit((unsigned char)*p)) return 0;
	while (p < end && isdigit((unsigned char)*p)) p++;

	if (p >= end || (*p != '.' && *p != ')')) return 0;
	p++;

	if (p >= end || *p != ' ') return 0;
	p++;

	*p_ptr = p;
	return 1;
}

static void flush_text(const char *start, const char *end)
{
	if (!start || !end || start >= end) return;

	MCNode_t *top_node = stack_peek(node_stack);
	MCNode_t *text_node = create_node(TEXT_NODE);
	if (!text_node) return;
	text_node->content = mc_span_range(start, end);
	add_child_node(top_node, text_node);
}

static MCNode_t *markcore_parse_link(const char **p_ptr, const char *end)
{
	const char *p = *p_ptr;
	const char *start = p;

	const char *close_bracket = seek_next_char(p, end, ']');
	if (!close_bracket) return NULL;

	p = close_bracket + 1;
	if (p >= end || *p != '(') return NULL;
	const char *open_link = p;

	const char *close_link = seek_next_char(p, end, ')');
	if (!close_link) return NULL;

	MCNode_t *link_node = create_node(LINK_NODE);
	if (!link_node) return NULL;
	link_node->content = mc_span_range(start + 1, close_bracket);
	link_node->data = mc_span_range(open_link + 1, close_link);

	*p_ptr = close_link + 1;
	return link_node;
}

static MCNode_t *markcore_parse_inline_code(const char **p_ptr, const char *end)
{
	const char *p = *p_ptr;
	const char *start = p;

	p++;
	const char *close_tick = seek_next_char(p, end, '`');
	if (!close_tick) return NULL;

	MCNode_t *node = create_node(CODE_INLINE_NODE);
	if (!node) return NULL;
	node->content = mc_span_range(start + 1, close_tick);

	*p_ptr = close_tick + 1;
	return node;
}

static int count_stars(const char *p, const char *end)
{
	int n = 0;
	while (p + n < end && p[n] == '*' && n < 3) n++;
	return n;
}

static MCNode_t *markcore_parse_italics_bold(const char **p_ptr, const char *end)
{
	const char *p = *p_ptr;
	const char *start = p;

	int delimiter_count = count_stars(p, end);
	if (delimiter_count == 0) return NULL;
	p += delimiter_count;

	const char *next_delimiter = seek_next_char(p, end, '*');
	if (!next_delimiter) return NULL;

	for (;;) {
		int close_count = count_stars(next_delimiter, end);
		if (close_count >= delimiter_count) break;
		next_delimiter = seek_next_char(next_delimiter + close_count, end, '*');
		if (!next_delimiter) return NULL;
	}

	MCNode_t *node = NULL;
	switch (delimiter_count) {
		case 1: node = create_node(ITALIC_NODE); break;
		case 2: node = create_node(BOLD_NODE); break;
		case 3: node = create_node(BOLD_ITALIC_NODE); break;
	}
	if (!node) return NULL;

	stack_push(node_stack, node);
	markcore_parse_inline_range(start + delimiter_count, next_delimiter);
	(void)stack_pop(node_stack);

	*p_ptr = next_delimiter + delimiter_count;
	return node;
}

static void markcore_parse_inline_range(const char *start, const char *end)
{
	const char *p = start;

	MCNode_t *top_node = stack_peek(node_stack);
	if (!top_node) return;

	const char *last_text = start;
	while (p < end) {
		const char *og_p = p;
		MCNode_t *new_node = NULL;
		switch (*p) {
		case '[':
			new_node = markcore_parse_link(&p, end);
			break;
		case '*':
			new_node = markcore_parse_italics_bold(&p, end);
			break;
		case '`':
			new_node = markcore_parse_inline_code(&p, end);
			break;
		default:
			break;
		}
		if (new_node) {
			flush_text(last_text, og_p);
			last_text = p;
			add_child_node(top_node, new_node);
			continue;
		}
		p++;
	}
	if (last_text < end) {
		flush_text(last_text, end);
	}
}

static MCNode_t *markcore_parse_image(const char *p, const char *end)
{
	const char *start = p;

	if (p + 1 >= end || p[1] != '[') return NULL;

	const char *close_bracket = seek_next_char(p + 1, end, ']');
	if (!close_bracket) return NULL;

	const char *open_link = close_bracket + 1;
	if (open_link >= end || *open_link != '(') return NULL;

	const char *close_link = seek_next_char(open_link, end, ')');
	if (!close_link) return NULL;

	MCNode_t *node = create_node(IMAGE_NODE);
	if (!node) return NULL;
	node->content = mc_span_range(start + 2, close_bracket);
	node->data = mc_span_range(open_link + 1, close_link);
	return node;
}

static MCNode_t *markcore_parse_header(const char *p, const char *end)
{
	int header_count = 0;
	while (p < end && *p == '#') {
		header_count++;
		p++;
	}

	if (header_count < 1 || header_count > 6) return NULL;

	if (p < end && *p == ' ') p++;

	MCNode_t *header_node = create_node(HEADER_NODE);
	if (!header_node) return NULL;
	header_node->content = mc_span_range(p, end);
	header_node->header_level = header_count;
	return header_node;
}

static void escape_if_in_list(MCNode_t **top_node)
{
	if (!top_node || !*top_node) return;
	if ((*top_node)->type == UNORDERED_LIST_NODE || (*top_node)->type == ORDERED_LIST_NODE) {
		(void)stack_pop(node_stack);
		*top_node = stack_peek(node_stack);
	}
}

static void ensure_list_context(MCNode_t **top_node, MCNodeType_e list_type)
{
	if (!top_node || !*top_node) return;
	if ((*top_node)->type == list_type) return;

	escape_if_in_list(top_node);
	if (!*top_node) return;

	MCNode_t *list_node = create_node(list_type);
	if (!list_node) return;
	add_child_node(*top_node, list_node);
	stack_push(node_stack, list_node);
	*top_node = list_node;
}

static void markcore_parse_line(const char *start, const char *end)
{
	const char *p = start;
	while (p < end && (*p == ' ' || *p == '\t')) p++;

	MCNode_t *top_node = stack_peek(node_stack);
	if (!top_node) {
		fprintf(stderr, "Error, stack is empty\n");
		return;
	}

	if (p >= end) {
		if (top_node->type == CODE_BLOCK_NODE) {
			MCNode_t *text_node = create_node(TEXT_NODE);
			add_child_node(top_node, text_node);
		}
		return;
	}

	if (top_node->type == CODE_BLOCK_NODE && !has_prefix(p, end, "```", 3)) {
		flush_text(start, end);
		return;
	}

	MCNode_t *temp_node;

	switch (*p) {
		case '#':
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_header(p, end);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '!':
			escape_if_in_list(&top_node);
			temp_node = markcore_parse_image(p, end);
			if (temp_node) {
				add_child_node(top_node, temp_node);
				return;
			}
			break;
		case '*':
			if (p + 1 < end && p[1] == ' ') {
				ensure_list_context(&top_node, UNORDERED_LIST_NODE);
				p += 2;
			}
			break;
		case '`':
			if (has_prefix(p, end, "```", 3)) {
				if (top_node->type != CODE_BLOCK_NODE) {
					escape_if_in_list(&top_node);
					MCNode_t *code_block_node = create_node(CODE_BLOCK_NODE);
					add_child_node(top_node, code_block_node);
					stack_push(node_stack, code_block_node);
				} else {
					(void)stack_pop(node_stack);
				}
				return;
			}
			break;
		default:
			if (is_ordered_list_item(&p, end)) {
				ensure_list_context(&top_node, ORDERED_LIST_NODE);
			} else {
				escape_if_in_list(&top_node);
			}
	}

	MCNode_t *line_node = create_node(LINE_NODE);
	if (!line_node) return;
	stack_push(node_stack, line_node);
	add_child_node(top_node, line_node);
	markcore_parse_inline_range(p, end);
	(void)stack_pop(node_stack);
}

void markcore_free_syntax_tree(MCNode_t *node)
{
	if (!node) return;

	for (int i = 0; i < node->child_count; i++) {
		markcore_free_syntax_tree(node->children[i]);
	}

	if (node->children) free(node->children);
	free(node);
}

void markcore_print_tree(MCNode_t *node, int depth)
{
	if (!node) return;

	int i;
	for (i = 0; i < depth; i++) {
		printf("\t");
	}

	if (depth > 0) {
		printf("└── ");
	}

	int clen = node->content.len > 2147483647 ? 2147483647 : (int)node->content.len;
	int dlen = node->data.len > 2147483647 ? 2147483647 : (int)node->data.len;
	const char *cptr = node->content.ptr ? node->content.ptr : "";
	const char *dptr = node->data.ptr ? node->data.ptr : "";

	switch (node->type) {
		case LINK_NODE:
			printf("Link – %.*s (%.*s)\n", clen, cptr, dlen, dptr);
			break;
		case HEADER_NODE:
			printf("Header %i\n", node->header_level);
			break;
		case TEXT_NODE:
			printf("Text – %.*s\n", clen, cptr);
			break;
		case IMAGE_NODE:
			printf("Image – %.*s\n", dlen, dptr);
			break;
		case CODE_INLINE_NODE:
			printf("Inline code – %.*s\n", clen, cptr);
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
