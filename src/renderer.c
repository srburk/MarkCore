
#include "renderer.h"
#include <stdio.h>

// Macros

#define SAFE_RENDER_CALL(r, fn, ...) \
    do { \
        if ((r)->fn) { \
            (r)->fn((r), ##__VA_ARGS__); \
        } else { \
            fprintf(stderr, "[Renderer warning] %s not implemented\n", #fn); \
        } \
    } while (0)

// default renderer

void renderer_destroy(Renderer_t *r) {
	if (!r) return;
	stack_free(r->node_stack);
	free(r);
}

static void traverse_children(Renderer_t *r, MCNode_t *node) {
	for (int i = 0; i < node->child_count; i++) {
		render_syntax_tree(r, node->children[i]);
	}
}

// Handlers ===========================================

// static void handle_line(Renderer_t *r, MCNode_t *node) {
// 
// 	MCNodeType_e *top_node_type = stack_peek(r->node_stack);
// 
// 	if (top_node_type && *top_node_type == UNORDERED_LIST_NODE) {
// 		SAFE_RENDER_CALL(r, render_list_item_open, r);
// 	}
// 	
// 	r->render_paragraph_open(r);
// 	traverse_children(r, node);
// 	r->render_paragraph_close(r);
// 	
// 	if (top_node_type && *top_node_type == UNORDERED_LIST_NODE) {
// 		r->render_list_item_close(r);
// 	}
// }

static void handle_list(Renderer_t *r, MCNode_t *node) {

	if (node->type != UNORDERED_LIST_NODE && node->type != ORDERED_LIST_NODE) {
		fprintf(stderr, "[Renderer] List handler passed wrong node type\n");
	}

	void (*render_list_open)(Renderer_t*) = (node->type == UNORDERED_LIST_NODE) ? r->render_unordered_list_open : r->render_ordered_list_open;
	void (*render_list_close)(Renderer_t*) = (node->type == UNORDERED_LIST_NODE) ? r->render_unordered_list_close : r->render_ordered_list_close;

	if (render_list_open) {
		render_list_open(r);
	} else {
		fprintf(stderr, "[Renderer warning] list_open not implemented\n");
	}
		
	MCNodeType_e list_node = node->type;
	stack_push(r->node_stack, &list_node);
	traverse_children(r, node);
	(void)stack_pop(r->node_stack);
	
	if (render_list_close) {
		render_list_close(r);
	} else {
		fprintf(stderr, "[Renderer warning] list_close not implemented\n");
	}
	
	SAFE_RENDER_CALL(r, render_line_end);
}

// using recursion here so I can swap in specific renderers
size_t render_syntax_tree(Renderer_t *r, MCNode_t *node) {

	if (!node) return 0;
	
	MCNodeType_e *top_node_type = stack_peek(r->node_stack);
	
	switch (node->type) {
		case LINE_NODE:
			if (top_node_type && (*top_node_type == UNORDERED_LIST_NODE || *top_node_type == ORDERED_LIST_NODE)) {
				SAFE_RENDER_CALL(r, render_list_item_open);
			}
			
			SAFE_RENDER_CALL(r, render_paragraph_open);
			traverse_children(r, node);
			SAFE_RENDER_CALL(r, render_paragraph_close);
			
			if (top_node_type && (*top_node_type == UNORDERED_LIST_NODE || *top_node_type == ORDERED_LIST_NODE)) {
				SAFE_RENDER_CALL(r, render_list_item_close);
			}
			
			SAFE_RENDER_CALL(r, render_line_end);
			
			break;
		case ROOT_NODE:
			traverse_children(r, node);
			break;
		
		case CODE_BLOCK_NODE:
			SAFE_RENDER_CALL(r, render_code_block_open); 
			MCNodeType_e code_block_node = CODE_BLOCK_NODE;
			stack_push(r->node_stack, &code_block_node);
			traverse_children(r, node);
			(void)stack_pop(r->node_stack);
			SAFE_RENDER_CALL(r, render_code_block_close); 
			break;	
		
		case CODE_INLINE_NODE:
			SAFE_RENDER_CALL(r, render_code_inline, node->content);
			break;
		
		case IMAGE_NODE:
			SAFE_RENDER_CALL(r, render_image, node->data, node->content);
			SAFE_RENDER_CALL(r, render_line_end);
			break;

		case TEXT_NODE: 
				
			if (top_node_type && *top_node_type == CODE_BLOCK_NODE) {
				SAFE_RENDER_CALL(r, render_code_block_line, node->content);
				SAFE_RENDER_CALL(r, render_line_end);
			} else {
				SAFE_RENDER_CALL(r, render_text, node->content);
			}
		
			break;
			
		case HEADER_NODE:
			SAFE_RENDER_CALL(r, render_header, node->header_level, node->content);
			SAFE_RENDER_CALL(r, render_line_end);
			break;
		
		case UNORDERED_LIST_NODE:
		case ORDERED_LIST_NODE:
			handle_list(r, node);
			break;
		
		case BOLD_NODE:
			SAFE_RENDER_CALL(r, render_bold_open); 
			traverse_children(r, node);
			SAFE_RENDER_CALL(r, render_bold_close); 
			break;
		case ITALIC_NODE:
			SAFE_RENDER_CALL(r, render_italic_open); 
			traverse_children(r, node);
			SAFE_RENDER_CALL(r, render_italic_close); 
			break;
		case BOLD_ITALIC_NODE:
			SAFE_RENDER_CALL(r, render_bold_open); 
			SAFE_RENDER_CALL(r, render_italic_open); 
			traverse_children(r, node);
			SAFE_RENDER_CALL(r, render_italic_close); 
			SAFE_RENDER_CALL(r, render_bold_close); 
			break;
		case LINK_NODE:
			SAFE_RENDER_CALL(r, render_link, node->data, node->content);
			break;
		default:
			printf("Not implemented renderer for: %s", type_labels[node->type]);
	}
	
	// for (int i = 0; i < node->child_count; i++) {
//     	if (node->children[i]) {
// 			render_syntax_tree(r, node->children[i]);
//     	}
// 	}
}