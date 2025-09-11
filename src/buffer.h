
#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

typedef struct {
	char *data;
	size_t used;
	size_t capacity;
} Buffer_t;

int buffer_init(Buffer_t *buffer, size_t initial);

int buffer_append(Buffer_t *buffer, const char *s, size_t s_len);

#endif