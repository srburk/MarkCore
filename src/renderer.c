
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

// using recursion here so I can swap in specific renderers
void render_syntax_tree(Renderer_t *r, MCNode_t *node) {

	if (!node) return;
	
	MCNodeType_e *top_node_type = stack_peek(r->node_stack);
	
	switch (node->type) {
		case LINE_NODE:
			if (top_node_type && *top_node_type == UNORDERED_LIST_NODE) {
				SAFE_RENDER_CALL(r, render_list_item_open);
			}
			
			SAFE_RENDER_CALL(r, render_paragraph_open);
			traverse_children(r, node);
			SAFE_RENDER_CALL(r, render_paragraph_close);
			
			if (top_node_type && *top_node_type == UNORDERED_LIST_NODE) {
				SAFE_RENDER_CALL(r, render_list_item_close);
			}
			
			SAFE_RENDER_CALL(r, render_line_end);
			
			break;
		case ROOT_NODE:
			traverse_children(r, node);
			break;
			
		case IMAGE_NODE:
			SAFE_RENDER_CALL(r, render_image, node->data, node->content);
			SAFE_RENDER_CALL(r, render_line_end);
			break;

		case TEXT_NODE: 
			SAFE_RENDER_CALL(r, render_text, node->content);
			break;
		case HEADER_NODE:
			SAFE_RENDER_CALL(r, render_header, node->header_level, node->content);
			SAFE_RENDER_CALL(r, render_line_end);
			break;
		
		case UNORDERED_LIST_NODE:
			SAFE_RENDER_CALL(r, render_unordered_list_open); 
			MCNodeType_e unordered_list_node = UNORDERED_LIST_NODE;
			stack_push(r->node_stack, &unordered_list_node);
			traverse_children(r, node);
			(void)stack_pop(r->node_stack);
			SAFE_RENDER_CALL(r, render_unordered_list_close); 
			SAFE_RENDER_CALL(r, render_line_end);
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