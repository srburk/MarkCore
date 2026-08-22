#ifndef MARKCORE_ARENA_H
#define MARKCORE_ARENA_H

#include <stddef.h>

typedef struct MCArena MCArena_t;

MCArena_t *mc_arena_create(void);
void *mc_arena_alloc(MCArena_t *arena, size_t size);
void *mc_arena_calloc(MCArena_t *arena, size_t size);
void mc_arena_destroy(MCArena_t *arena);

#endif
