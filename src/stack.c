
#include "stack.h"

Stack_t *stack_create(size_t capacity) {
    Stack_t *s = malloc(sizeof(Stack_t));
    s->items = malloc(sizeof(void*) * capacity);
    s->size = 0;
    s->capacity = capacity;
    return s;
}

void stack_push(Stack_t *s, void *item) {
    if (s->size >= s->capacity) {
        s->capacity *= 2;
        s->items = realloc(s->items, sizeof(void*) * s->capacity);
    }
    s->items[s->size++] = item;
}

void *stack_pop(Stack_t *s) {
    if (s->size == 0) return NULL;
    return s->items[--s->size];
}

void *stack_peek(Stack_t *s) {
    if (s->size == 0) return NULL;
    return s->items[s->size - 1];
}

void stack_free(Stack_t *s) {
    free(s->items);
    free(s);
}

void stack_print(Stack_t *s, void (*item_print)(void *item)) {
	if (s->size == 0) {
		printf("Stack empty\n");
	} else {
		printf("Stack: ");
		for (int i = 0; i < s->size; i ++) {
			item_print(s->items[i]);
			if (i < s->size - 1) printf(" -> ");
		}
		printf("\n");
	}
}