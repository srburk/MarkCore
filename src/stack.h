
#ifndef MARKCORE_STACK_H
#define MARKCORE_STACK_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	void **items;
	size_t size;
	size_t capacity;
} Stack_t;

Stack_t *stack_create(size_t capacity);
void stack_push(Stack_t *s, void *item);
void *stack_pop(Stack_t *s);
void *stack_peek(Stack_t *s);
void stack_free(Stack_t *s);

void stack_print(Stack_t *s, void (*print_func)(void *item));

#endif