
#include "buffer.h"

int buffer_init(Buffer_t *buffer, size_t initial) {
	buffer->data = malloc(initial);
	if (!buffer->data) return -1;
	buffer->used = 0;
	buffer->capacity = initial;
	return 0;
}

int buffer_append(Buffer_t *buffer, const char *s, size_t s_len) {
	// grow buffer if needed
	if (buffer->used + s_len + 1 > buffer->capacity) {
		size_t new_size = buffer->capacity * 2;
        while (buffer->used + s_len + 1 > new_size) {
            new_size *= 2;
        }
        char *new_data = realloc(buffer->data, new_size);
        if (!new_data) return -1;
        buffer->data = new_data;
        buffer->capacity = new_size;
	}
	strncpy(buffer->data, s, s_len);
	buffer->used += s_len;
	buffer->data[buffer->used] = '\0';
	return 0;
}