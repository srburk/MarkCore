#include "markcore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static char *render_to_string(const char *markdown) {
	char *buf = NULL;
	size_t size = 0;
	FILE *mem = open_memstream(&buf, &size);
	if (!mem) return NULL;
	markcore_render_to_file(markdown, strlen(markdown), mem);
	fclose(mem);
	return buf;
}

static void expect_contains(const char *name, const char *haystack, const char *needle) {
	if (!haystack || !strstr(haystack, needle)) {
		fprintf(stderr, "FAIL %s: expected to contain %s\n  got: %s\n",
			name, needle, haystack ? haystack : "(null)");
		failures++;
	}
}

static void expect_not_contains(const char *name, const char *haystack, const char *needle) {
	if (haystack && strstr(haystack, needle)) {
		fprintf(stderr, "FAIL %s: expected NOT to contain %s\n  got: %s\n",
			name, needle, haystack);
		failures++;
	}
}

int main(void) {
	char *out;

	/* Second parse must not double-free the global node stack. */
	out = render_to_string("hello\n");
	free(out);
	out = render_to_string("hello\n");
	expect_contains("double-parse", out, "<p>hello</p>");
	free(out);

	/* Character after an inline construct must still be parsed as markup. */
	out = render_to_string("`code`*italic*");
	expect_contains("adjacent-inline-em", out, "<code>code</code>");
	expect_contains("adjacent-inline-em", out, "<em>italic</em>");
	free(out);

	out = render_to_string("`x`[link](http://y)");
	expect_contains("adjacent-inline-link", out, "<code>x</code>");
	expect_contains("adjacent-inline-link", out, "<a href=\"http://y\">link</a>");
	free(out);

	/* Switching list types should produce sibling lists, not nesting. */
	out = render_to_string("* a\n1. b\n");
	expect_contains("list-switch-ul", out, "<ul>");
	expect_contains("list-switch-ol", out, "<ol>");
	expect_not_contains("list-switch-nested", out, "<ul><li>a</li>\n<ol>");
	free(out);

	/* Blank lines inside fenced code must be preserved. */
	out = render_to_string("```\nline1\n\nline3\n```\n");
	expect_contains("code-blank", out, "line1\n\nline3\n");
	free(out);

	/* HTML in markdown text must be escaped. */
	out = render_to_string("<script>alert(1)</script>\n");
	expect_contains("html-escape", out, "&lt;script&gt;");
	expect_not_contains("html-escape", out, "<script>");
	free(out);

	/* ATX headings are 1–6; seven hashes is not a heading. */
	out = render_to_string("####### seven\n");
	expect_not_contains("h7", out, "<h7>");
	expect_contains("h7-paragraph", out, "####### seven");
	free(out);

	out = render_to_string("# Title\n");
	expect_contains("h1-no-leading-space", out, "<h1>Title</h1>");
	free(out);

	/* List markers should not leave a leading space in item text. */
	out = render_to_string("* item\n");
	expect_contains("ul-no-leading-space", out, "<li>item</li>");
	expect_not_contains("ul-no-leading-space", out, "<p>item</p>");
	expect_not_contains("ul-no-leading-space", out, "<li> item</li>");
	free(out);

	/* Quotes in attributes must be escaped; apostrophes in text need not. */
	out = render_to_string("I'm a [test](http://x.com/\"q\")\n");
	expect_contains("attr-escape", out, "I'm a");
	expect_contains("attr-escape", out, "&quot;");
	free(out);

	if (failures) {
		fprintf(stderr, "%d test(s) failed\n", failures);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
