
#include "html_renderer.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Forward Declaration ======================================

static size_t html_render_header(Renderer_t *r, int header_level, const char *text);
static size_t html_render_text(Renderer_t *r,  const char *text);
static size_t html_render_image(Renderer_t *r, const char *url, const char *alt);
static size_t html_render_link(Renderer_t *r, const char *url, const char *text);

static size_t html_render_line_end(Renderer_t* r);

static size_t html_render_paragraph_open(Renderer_t *r);
static size_t html_render_paragraph_close(Renderer_t *r);

static size_t html_render_code_block_open(Renderer_t *r);
static size_t html_render_code_block_close(Renderer_t *r);
static size_t html_render_code_block_line(Renderer_t *r, const char *text);
static size_t html_render_code_inline(Renderer_t *r, const char *text);

static size_t html_render_bold_open(Renderer_t *r);
static size_t html_render_bold_close(Renderer_t *r);

static size_t html_render_italic_open(Renderer_t *r);
static size_t html_render_italic_close(Renderer_t *r);

static size_t html_render_unordered_list_open(Renderer_t *r);
static size_t html_render_unordered_list_close(Renderer_t *r);
static size_t html_render_ordered_list_open(Renderer_t *r);
static size_t html_render_ordered_list_close(Renderer_t *r);

static size_t html_render_list_item_open(Renderer_t *r);
static size_t html_render_list_item_close(Renderer_t *r);

// Renderer =================================================

Renderer_t *create_html_renderer(FILE *dest) {
	
	Renderer_t *r = malloc(sizeof(Renderer_t));
	if (!r) return NULL;
	
	r->outfile = dest;
	r->node_stack = stack_create(4);
	if (!r->node_stack) {
		free(r);
		return NULL;
	}
	
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

static size_t html_emit(FILE *outfile, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vfprintf(outfile, fmt, args);
    va_end(args);

    return (written < 0) ? 0 : (size_t)written;
}

static size_t html_emit_escaped(FILE *outfile, const char *text, int is_attr) {
	if (!text) return 0;
	size_t written = 0;
	for (const char *p = text; *p; p++) {
		const char *rep = NULL;
		switch (*p) {
			case '&':  rep = "&amp;"; break;
			case '<':  rep = "&lt;"; break;
			case '>':  rep = "&gt;"; break;
			case '"':  if (is_attr) { rep = "&quot;"; break; } goto emit_raw;
			case '\'': if (is_attr) { rep = "&#39;"; break; } goto emit_raw;
			default:
			emit_raw:
				if (fputc(*p, outfile) != EOF) written += 1;
				continue;
		}
		if (fputs(rep, outfile) != EOF) written += strlen(rep);
	}
	return written;
}

// Renderer Functions ==============================================

static size_t html_render_header(Renderer_t *r, int header_level, const char *text) {
	if (header_level < 1) header_level = 1;
	if (header_level > 6) header_level = 6;
	size_t written = html_emit(r->outfile, "<h%i>", header_level);
	written += html_emit_escaped(r->outfile, text, 0);
	written += html_emit(r->outfile, "</h%i>", header_level);
	return written;
}

static size_t html_render_text(Renderer_t *r,  const char *text) {
	return html_emit_escaped(r->outfile, text, 0);
}

static size_t html_render_image(Renderer_t *r, const char *url, const char *alt) {
	size_t written = html_emit(r->outfile, "<img src=\"");
	written += html_emit_escaped(r->outfile, url, 1);
	written += html_emit(r->outfile, "\" alt=\"");
	written += html_emit_escaped(r->outfile, alt, 1);
	written += html_emit(r->outfile, "\" />");
	return written;
}

static size_t html_render_link(Renderer_t *r, const char *url, const char *text) {
	size_t written = html_emit(r->outfile, "<a href=\"");
	written += html_emit_escaped(r->outfile, url, 1);
	written += html_emit(r->outfile, "\">");
	written += html_emit_escaped(r->outfile, text, 0);
	written += html_emit(r->outfile, "</a>");
	return written;
}

static size_t html_render_paragraph_open(Renderer_t *r) {
	return html_emit(r->outfile, "<p>");
}

static size_t html_render_paragraph_close(Renderer_t *r) {
	return html_emit(r->outfile, "</p>");
}

static size_t html_render_code_block_open(Renderer_t *r) {
	return html_emit(r->outfile, "<pre><code>");
}

static size_t html_render_code_block_close(Renderer_t *r) {
	return html_emit(r->outfile, "</code></pre>");
}

static size_t html_render_code_block_line(Renderer_t *r, const char *text) {
	return html_emit_escaped(r->outfile, text, 0);
}

static size_t html_render_code_inline(Renderer_t *r, const char *text) {
	size_t written = html_emit(r->outfile, "<code>");
	written += html_emit_escaped(r->outfile, text, 0);
	written += html_emit(r->outfile, "</code>");
	return written;
}

static size_t html_render_line_end(Renderer_t* r) {
	return html_emit(r->outfile, "\n");
}

static size_t html_render_bold_open(Renderer_t *r) {
	return html_emit(r->outfile, "<strong>");
}

static size_t html_render_bold_close(Renderer_t *r) {
	return html_emit(r->outfile, "</strong>");
}

static size_t html_render_italic_open(Renderer_t *r) {
	return html_emit(r->outfile, "<em>");
}

static size_t html_render_italic_close(Renderer_t *r) {
	return html_emit(r->outfile, "</em>");
}

static size_t html_render_unordered_list_open(Renderer_t *r) {
	return html_emit(r->outfile, "<ul>");
}

static size_t html_render_unordered_list_close(Renderer_t *r) {
	return html_emit(r->outfile, "</ul>");
}

static size_t html_render_ordered_list_open(Renderer_t *r) {
	return html_emit(r->outfile, "<ol>");
}

static size_t html_render_ordered_list_close(Renderer_t *r) {
	return html_emit(r->outfile, "</ol>");
}

static size_t html_render_list_item_open(Renderer_t *r) {
	return html_emit(r->outfile, "<li>");
}

static size_t html_render_list_item_close(Renderer_t *r) {
	return html_emit(r->outfile, "</li>");
}