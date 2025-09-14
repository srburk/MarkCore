
#include "html_renderer.h"

// Forward Declaration ======================================

static void html_render_header(Renderer_t *r, int header_level, const char *text);
static void html_render_text(Renderer_t *r,  const char *text);
static void html_render_image(Renderer_t *r, const char *url, const char *alt);
static void html_render_link(Renderer_t *r, const char *url, const char *text);

static void html_render_line_end(Renderer_t* r);

static void html_render_paragraph_open(Renderer_t *r);
static void html_render_paragraph_close(Renderer_t *r);

static void html_render_code_block_open(Renderer_t *r);
static void html_render_code_block_close(Renderer_t *r);
static void html_render_code_block_line(Renderer_t *r, const char *text);
static void html_render_code_inline(Renderer_t *r, const char *text);

static void html_render_bold_open(Renderer_t *r);
static void html_render_bold_close(Renderer_t *r);

static void html_render_italic_open(Renderer_t *r);
static void html_render_italic_close(Renderer_t *r);

static void html_render_unordered_list_open(Renderer_t *r);
static void html_render_unordered_list_close(Renderer_t *r);
static void html_render_ordered_list_open(Renderer_t *r);
static void html_render_ordered_list_close(Renderer_t *r);

static void html_render_list_item_open(Renderer_t *r);
static void html_render_list_item_close(Renderer_t *r);

// Renderer =================================================

Renderer_t *create_html_renderer(FILE *dest) {
	
	Renderer_t *r = malloc(sizeof(Renderer_t));
	if (!r) return NULL;
	
	r->outfile = dest;
	r->node_stack = stack_create(4);
	
	r->render_header = html_render_header;
	r->render_text = html_render_text;
	r->render_image = html_render_image;
	r->render_link = html_render_link;
	
	r->render_line_end = html_render_line_end;
	
	r->render_paragraph_open = html_render_paragraph_open;
	r->render_paragraph_close = html_render_paragraph_close;
	
	r->render_code_block_open = html_render_code_block_open;
	r->render_code_block_close = html_render_code_block_close;
	r->render_code_block_line = html_render_code_block_line;
	
	r->render_code_inline = html_render_code_inline;

	r->render_bold_open = html_render_bold_open;
	r->render_bold_close = html_render_bold_close;
	
	r->render_italic_open = html_render_italic_open;
	r->render_italic_close = html_render_italic_close;
	
	r->render_unordered_list_open = html_render_unordered_list_open;
	r->render_unordered_list_close = html_render_unordered_list_close;
	r->render_ordered_list_open = html_render_ordered_list_open;
	r->render_ordered_list_close = html_render_ordered_list_close;
	
	r->render_list_item_open = html_render_list_item_open;
	r->render_list_item_close = html_render_list_item_close;
	
	return r;
}

static void html_render_header(Renderer_t *r, int header_level, const char *text) {
	fprintf(r->outfile, "<h%i>%s</h%i>", header_level, text, header_level);
}

static void html_render_text(Renderer_t *r,  const char *text) {
	fprintf(r->outfile, "%s", text);
}

static void html_render_image(Renderer_t *r, const char *url, const char *alt) {
	fprintf(r->outfile, "<img src=\"%s\" alt=\"%s\" />", url, alt);
}

static void html_render_link(Renderer_t *r, const char *url, const char *text) {
	fprintf(r->outfile, "<a href=\"%s\">%s</a>", url, text);
}

static void html_render_paragraph_open(Renderer_t *r) {
	fprintf(r->outfile, "<p>");
}

static void html_render_paragraph_close(Renderer_t *r) {
	fprintf(r->outfile, "</p>");
}

static void html_render_code_block_open(Renderer_t *r) {
	fprintf(r->outfile, "<pre><code>");
}

static void html_render_code_block_close(Renderer_t *r) {
	fprintf(r->outfile, "</code></pre>");
}

static void html_render_code_block_line(Renderer_t *r, const char *text) {
	// no formatting
	for (const char *p = text; *p; p++) {
		switch (*p) {
            case '&': fputs("&amp;", r->outfile); break;
            case '<': fputs("&lt;", r->outfile); break;
            case '>': fputs("&gt;", r->outfile); break;
            case '"': fputs("&quot;", r->outfile); break;
            case '\'': fputs("&#39;", r->outfile); break;
            default: fputc(*p, r->outfile); break;
        }
	}
}

static void html_render_code_inline(Renderer_t *r, const char *text) {
	fprintf(r->outfile, "<code>%s</code>", text);
}

static void html_render_line_end(Renderer_t* r) {
	fprintf(r->outfile, "\n");
}

static void html_render_bold_open(Renderer_t *r) {
	fprintf(r->outfile, "<strong>");
}

static void html_render_bold_close(Renderer_t *r) {
	fprintf(r->outfile, "</strong>");
}

static void html_render_italic_open(Renderer_t *r) {
	fprintf(r->outfile, "<em>");
}

static void html_render_italic_close(Renderer_t *r) {
	fprintf(r->outfile, "</em>");
}

static void html_render_unordered_list_open(Renderer_t *r) {
	fprintf(r->outfile, "<ul>");
}

static void html_render_unordered_list_close(Renderer_t *r) {
	fprintf(r->outfile, "</ul>");
}

static void html_render_ordered_list_open(Renderer_t *r) {
	fprintf(r->outfile, "<ol>");
}

static void html_render_ordered_list_close(Renderer_t *r) {
	fprintf(r->outfile, "</ol>");
}

static void html_render_list_item_open(Renderer_t *r) {
	fprintf(r->outfile, "<li>");
}

static void html_render_list_item_close(Renderer_t *r) {
	fprintf(r->outfile, "</li>");
}