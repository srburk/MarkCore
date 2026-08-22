/*
 * Generate three classes of markdown and time markcore_render_to_file.
 *
 * Usage:
 *   markcore-bench              # print per-class timing + allocs
 *   markcore-bench --ci         # same, then fail if over CI ceilings
 *
 * When linked with -Wl,--wrap=malloc (see CMakeLists), allocation counts
 * are recorded. Without wrap support, alloc columns print as 0.
 */

#include "markcore.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
	char *data;
	size_t len;
	size_t cap;
} Buf;

typedef struct {
	uint32_t s;
} Rng;

static uint64_t g_alloc_count;
static uint64_t g_alloc_bytes;
static int g_count_allocs;

#if defined(MARKCORE_BENCH_WRAP_ALLOC)
void *__real_malloc(size_t);
void *__real_calloc(size_t, size_t);
void *__real_realloc(void *, size_t);
void __real_free(void *);

void *__wrap_malloc(size_t n)
{
	if (g_count_allocs) {
		g_alloc_count++;
		g_alloc_bytes += n;
	}
	return __real_malloc(n);
}

void *__wrap_calloc(size_t nmemb, size_t size)
{
	if (g_count_allocs) {
		g_alloc_count++;
		g_alloc_bytes += nmemb * size;
	}
	return __real_calloc(nmemb, size);
}

void *__wrap_realloc(void *p, size_t n)
{
	if (g_count_allocs) {
		g_alloc_count++;
		g_alloc_bytes += n;
	}
	return __real_realloc(p, n);
}

void __wrap_free(void *p)
{
	__real_free(p);
}
#endif

static uint32_t rng_next(Rng *r)
{
	uint32_t x = r->s;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return r->s = x ? x : 0xA5A5A5A5u;
}

static uint32_t rng_range(Rng *r, uint32_t lo, uint32_t hi)
{
	return lo + (rng_next(r) % (hi - lo + 1));
}

static void buf_grow(Buf *b, size_t extra)
{
	if (b->len + extra <= b->cap) return;
	size_t cap = b->cap ? b->cap : 4096;
	while (b->len + extra > cap) cap *= 2;
	char *n = realloc(b->data, cap);
	if (!n) {
		fprintf(stderr, "out of memory generating markdown\n");
		exit(1);
	}
	b->data = n;
	b->cap = cap;
}

static void buf_put(Buf *b, const char *s)
{
	size_t n = strlen(s);
	buf_grow(b, n);
	memcpy(b->data + b->len, s, n);
	b->len += n;
}

static void buf_putc(Buf *b, char c)
{
	buf_grow(b, 1);
	b->data[b->len++] = c;
}

static const char *k_words[] = {
	"the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog",
	"markdown", "parser", "renders", "html", "heading", "list", "item",
	"code", "block", "inline", "link", "image", "bold", "italic",
	"document", "buffer", "span", "node", "tree", "stack", "token",
	"alpha", "beta", "gamma", "delta", "omega", "value", "count",
};
static const size_t k_nwords = sizeof(k_words) / sizeof(k_words[0]);

static void put_word(Buf *b, Rng *r)
{
	buf_put(b, k_words[rng_next(r) % k_nwords]);
}

/* Class 1: long prose, sparse markup — few nodes, large text spans. */
static void gen_prose(Buf *b, Rng *r)
{
	for (int para = 0; para < 4000; para++) {
		int words = (int)rng_range(r, 30, 70);
		for (int w = 0; w < words; w++) {
			if (w) buf_putc(b, ' ');
			uint32_t roll = rng_next(r) % 40;
			if (roll == 0) {
				buf_put(b, "**");
				put_word(b, r);
				buf_put(b, "**");
			} else if (roll == 1) {
				buf_put(b, "*");
				put_word(b, r);
				buf_put(b, "*");
			} else if (roll == 2) {
				buf_put(b, "[");
				put_word(b, r);
				buf_put(b, "](https://example.com/");
				put_word(b, r);
				buf_put(b, ")");
			} else {
				put_word(b, r);
			}
		}
		buf_put(b, "\n\n");
	}
}

/* Class 2: dense inlines — many small nodes (worst case for strdup). */
static void gen_inline(Buf *b, Rng *r)
{
	for (int line = 0; line < 6000; line++) {
		int n = (int)rng_range(r, 12, 20);
		for (int i = 0; i < n; i++) {
			if (i) buf_putc(b, ' ');
			switch (rng_next(r) % 5) {
			case 0:
				buf_put(b, "**");
				put_word(b, r);
				buf_put(b, "**");
				break;
			case 1:
				buf_put(b, "*");
				put_word(b, r);
				buf_put(b, "*");
				break;
			case 2:
				buf_putc(b, '`');
				put_word(b, r);
				buf_putc(b, '`');
				break;
			case 3:
				buf_put(b, "[");
				put_word(b, r);
				buf_put(b, "](https://ex.test/");
				put_word(b, r);
				buf_put(b, ")");
				break;
			default:
				put_word(b, r);
				break;
			}
		}
		buf_putc(b, '\n');
	}
}

/* Class 3: block structure — headings, lists, fenced code. */
static void gen_blocks(Buf *b, Rng *r)
{
	for (int sec = 0; sec < 800; sec++) {
		int level = (int)rng_range(r, 1, 4);
		for (int i = 0; i < level; i++) buf_putc(b, '#');
		buf_putc(b, ' ');
		put_word(b, r);
		buf_putc(b, ' ');
		put_word(b, r);
		buf_putc(b, '\n');

		int items = (int)rng_range(r, 4, 10);
		for (int i = 0; i < items; i++) {
			buf_put(b, "* ");
			put_word(b, r);
			buf_putc(b, ' ');
			if (rng_next(r) % 3 == 0) {
				buf_put(b, "**");
				put_word(b, r);
				buf_put(b, "**");
			} else {
				put_word(b, r);
			}
			buf_putc(b, '\n');
		}

		items = (int)rng_range(r, 3, 7);
		for (int i = 0; i < items; i++) {
			char num[16];
			snprintf(num, sizeof(num), "%d. ", i + 1);
			buf_put(b, num);
			put_word(b, r);
			buf_putc(b, ' ');
			put_word(b, r);
			buf_putc(b, '\n');
		}

		buf_put(b, "```\n");
		int nlines = (int)rng_range(r, 4, 12);
		for (int i = 0; i < nlines; i++) {
			int words = (int)rng_range(r, 4, 10);
			for (int w = 0; w < words; w++) {
				if (w) buf_putc(b, ' ');
				put_word(b, r);
			}
			buf_putc(b, '\n');
		}
		buf_put(b, "```\n\n");
	}
}

typedef void (*GenFn)(Buf *, Rng *);

typedef struct {
	const char *name;
	GenFn gen;
	uint32_t seed;
	/* CI ceilings: chunk/stack mallocs after the AST arena. */
	uint64_t max_allocs_per_iter;
	double max_ms_total;
} Class;

/*
 * Allocation ceilings after the AST arena: a handful of chunk/stack
 * mallocs per parse, not per-node. Keep these well below the span-only
 * baseline (67k / 257k / 49k) so a regression to calloc-per-node fails.
 */
static Class k_classes[] = {
	{ "prose",  gen_prose,  0xC0FFEE01u,  200, 3000.0 },
	{ "inline", gen_inline, 0xC0FFEE02u,  200, 3000.0 },
	{ "blocks", gen_blocks, 0xC0FFEE03u,  200, 3000.0 },
};

static double elapsed_ms(struct timespec a, struct timespec b)
{
	return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

static int run_class(Class *c, int warmup, int iters, FILE *nullf, int ci)
{
	Buf buf = {0};
	Rng rng = { c->seed };
	c->gen(&buf, &rng);
	buf_putc(&buf, '\0');
	size_t md_len = buf.len - 1;

	g_count_allocs = 0;
	for (int i = 0; i < warmup; i++) {
		markcore_render_to_file(buf.data, md_len, nullf);
	}

	g_alloc_count = 0;
	g_alloc_bytes = 0;
	g_count_allocs = 1;

	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (int i = 0; i < iters; i++) {
		markcore_render_to_file(buf.data, md_len, nullf);
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	g_count_allocs = 0;

	double ms = elapsed_ms(t0, t1);
	double mb = (double)md_len * (double)iters / (1024.0 * 1024.0);
	double mbps = (ms > 0) ? (mb / (ms / 1000.0)) : 0;
	uint64_t allocs_per = (uint64_t)iters ? g_alloc_count / (uint64_t)iters : 0;
	uint64_t bytes_per = (uint64_t)iters ? g_alloc_bytes / (uint64_t)iters : 0;

	printf("%-8s  %7.1f KB  %4d iters  %8.1f ms  %7.1f MB/s  %8llu allocs/iter  %7.1f KB alloc/iter\n",
		c->name,
		md_len / 1024.0,
		iters,
		ms,
		mbps,
		(unsigned long long)allocs_per,
		bytes_per / 1024.0);

	int failed = 0;
	if (ci) {
		if (allocs_per > c->max_allocs_per_iter) {
			fprintf(stderr, "CI FAIL %s: allocs/iter %llu > ceiling %llu\n",
				c->name,
				(unsigned long long)allocs_per,
				(unsigned long long)c->max_allocs_per_iter);
			failed = 1;
		}
		if (ms > c->max_ms_total) {
			fprintf(stderr, "CI FAIL %s: %.1f ms > ceiling %.1f ms\n",
				c->name, ms, c->max_ms_total);
			failed = 1;
		}
	}

	free(buf.data);
	return failed;
}

int main(int argc, char **argv)
{
	int ci = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ci") == 0) ci = 1;
	}

	FILE *nullf = fopen("/dev/null", "w");
	if (!nullf) {
		fprintf(stderr, "could not open /dev/null\n");
		return 1;
	}

	const int warmup = 1;
	const int iters = ci ? 4 : 8;

	printf("class       input     iters      time    throughput         allocs              bytes\n");
	int failed = 0;
	for (size_t i = 0; i < sizeof(k_classes) / sizeof(k_classes[0]); i++) {
		failed |= run_class(&k_classes[i], warmup, iters, nullf, ci);
	}

	fclose(nullf);
	return failed ? 1 : 0;
}
