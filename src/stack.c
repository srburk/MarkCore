
#include "stack.h"

Stack_t *stack_create(size_t capacity) {
	if (capacity == 0) capacity = 1;
	Stack_t *s = malloc(sizeof(Stack_t));
	if (!s) return NULL;
	s->items = malloc(sizeof(void *) * capacity);
	if (!s->items) {
		free(s);
		return NULL;
	}
	s->size = 0;
	s->capacity = capacity;
	return s;
}

void stack_push(Stack_t *s, void *item) {
	if (!s) return;
	if (s->size >= s->capacity) {
		size_t new_capacity = s->capacity * 2;
		void **new_items = realloc(s->items, sizeof(void *) * new_capacity);
		if (!new_items) return;
		s->items = new_items;
		s->capacity = new_capacity;
	}
	s->items[s->size++] = item;
}

void *stack_pop(Stack_t *s) {
	if (!s || s->size == 0) return NULL;
	return s->items[--s->size];
}

void *stack_peek(Stack_t *s) {
	if (!s || s->size == 0) return NULL;
	return s->items[s->size - 1];
}

void stack_free(Stack_t *s) {
	if (!s) return;
	free(s->items);
	free(s);
}

void stack_print(Stack_t *s, void (*item_print)(void *item)) {
	if (!s || s->size == 0) {
		printf("Stack empty\n");
	} else {
		printf("Stack: ");
		for (size_t i = 0; i < s->size; i++) {
			item_print(s->items[i]);
			if (i < s->size - 1) printf(" -> ");
		}
		printf("\n");
	}
}
