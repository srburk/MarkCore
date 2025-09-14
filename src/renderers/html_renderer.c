
#include "html_renderer.h"

// Forward Declaration ======================================

static void html_render_header(Renderer_t *r, int header_level, const char *text);
static void html_render_text(Renderer_t *r,  const char *text);
static void html_render_image(Renderer_t *r, const char *url, const char *alt);
static void html_render_link(Renderer_t *r, const char *url, const char *text);

static void html_render_line_end(Renderer_t* r);

static void html_render_paragraph_open(Renderer_t *r);
static void html_render_paragraph_close(Renderer_t *r);

static void html_render_bold_open(Renderer_t *r);
static void html_render_bold_close(Renderer_t *r);

static void html_render_italic_open(Renderer_t *r);
static void html_render_italic_close(Renderer_t *r);

static void html_render_unordered_list_open(Renderer_t *r);
static void html_render_unordered_list_close(Renderer_t *r);

static void html_render_list_item_open(Renderer_t *r);
static void html_render_list_item_close(Renderer_t *r);

// Renderer =================================================

Renderer_t create_html_renderer(FILE *dest) {
	
	Renderer_t renderer = {};
	
	renderer.outfile = dest;
	renderer.node_stack = stack_create(4);
	
	renderer.render_header = html_render_header;
	renderer.render_text = html_render_text;
	renderer.render_image = html_render_image;
	renderer.render_link = html_render_link;
	
	renderer.render_line_end = html_render_line_end;
	
	renderer.render_paragraph_open = html_render_paragraph_open;
	renderer.render_paragraph_close = html_render_paragraph_close;

	renderer.render_bold_open = html_render_bold_open;
	renderer.render_bold_close = html_render_bold_close;
	
	renderer.render_italic_open = html_render_italic_open;
	renderer.render_italic_close = html_render_italic_close;
	
	renderer.render_unordered_list_open = html_render_unordered_list_open;
	renderer.render_unordered_list_close = html_render_unordered_list_close;
	
	renderer.render_list_item_open = html_render_list_item_open;
	renderer.render_list_item_close = html_render_list_item_close;
	
	return renderer;
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

static void html_render_list_item_open(Renderer_t *r) {
	fprintf(r->outfile, "<li>");
}

static void html_render_list_item_close(Renderer_t *r) {
	fprintf(r->outfile, "</li>");
}