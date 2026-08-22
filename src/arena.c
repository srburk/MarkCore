#include "arena.h"

#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MC_ARENA_FIRST_CHUNK ((size_t)64 * 1024)
#define MC_ARENA_MAX_CHUNK   ((size_t)1024 * 1024)

typedef struct MCArenaChunk {
	struct MCArenaChunk *next;
	size_t cap;
	size_t used;
	_Alignas(max_align_t) unsigned char data[];
} MCArenaChunk_t;

struct MCArena {
	MCArenaChunk_t *first;
	MCArenaChunk_t *current;
	size_t next_chunk_size;
};

static size_t arena_align(size_t n)
{
	size_t a = alignof(max_align_t);
	return (n + a - 1) & ~(a - 1);
}

static MCArenaChunk_t *arena_new_chunk(size_t cap)
{
	MCArenaChunk_t *chunk = malloc(sizeof(MCArenaChunk_t) + cap);
	if (!chunk) return NULL;
	chunk->next = NULL;
	chunk->cap = cap;
	chunk->used = 0;
	return chunk;
}

MCArena_t *mc_arena_create(void)
{
	MCArena_t *arena = calloc(1, sizeof(*arena));
	if (!arena) return NULL;
	arena->next_chunk_size = MC_ARENA_FIRST_CHUNK;
	return arena;
}

void *mc_arena_alloc(MCArena_t *arena, size_t size)
{
	if (!arena || size == 0) return NULL;

	size = arena_align(size);

	if (arena->current && arena->current->used + size <= arena->current->cap) {
		void *p = arena->current->data + arena->current->used;
		arena->current->used += size;
		return p;
	}

	size_t cap = arena->next_chunk_size;
	if (size > cap) cap = size;

	MCArenaChunk_t *chunk = arena_new_chunk(cap);
	if (!chunk) return NULL;

	if (arena->current) {
		arena->current->next = chunk;
	} else {
		arena->first = chunk;
	}
	arena->current = chunk;

	if (arena->next_chunk_size < MC_ARENA_MAX_CHUNK) {
		size_t grown = arena->next_chunk_size * 2;
		arena->next_chunk_size = grown > MC_ARENA_MAX_CHUNK ? MC_ARENA_MAX_CHUNK : grown;
	}

	chunk->used = size;
	return chunk->data;
}

void *mc_arena_calloc(MCArena_t *arena, size_t size)
{
	void *p = mc_arena_alloc(arena, size);
	if (p) memset(p, 0, arena_align(size));
	return p;
}

void mc_arena_destroy(MCArena_t *arena)
{
	if (!arena) return;

	MCArenaChunk_t *chunk = arena->first;
	while (chunk) {
		MCArenaChunk_t *next = chunk->next;
		free(chunk);
		chunk = next;
	}
	free(arena);
}
