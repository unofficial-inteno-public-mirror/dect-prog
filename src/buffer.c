#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "error.h"


buffer_t * buffer_new(int size) {

	buffer_t *b = (buffer_t *) calloc(sizeof(buffer_t), 1);

	if (!b) {
		exit_failure("malloc\n");
	}
	
	b->in = calloc(size, 1);
	if (!b->in) {
		exit_failure("malloc\n");
	}
	
	b->count = 0;
	b->cursor = 0;
	b->max = size;

	return b;
}


int buffer_write(buffer_t * self, uint8_t *input, int count) {

	/* Don't write beyond buffer boundary */
	if ( self->count + count > self->max) {
		count = self->max - self->count;
	}

	memcpy(self->in + self->count, input, count);
	self->count += count;
	
	return count;
}


int buffer_read(buffer_t * self, uint8_t *buf, int count) {

	/* Don't read beyond data boundary */
	if ( self->cursor + count > self->count) {
		count = self->count - self->cursor;
	}

	memcpy(buf, self->in + self->cursor, count);
	self->cursor += count;
	
	return count;
}


int buffer_rewind(buffer_t * self, int count) {
	
	/* Don't rewind beyond start of buffer */
	if ( self->cursor - count < 0 ) {
		self->cursor = 0;
	} else {
		self->cursor -= count;
	}
}


int buffer_find(buffer_t * self, uint8_t c) {
	
	int i;

	/* Do we have byte c in buffer? */
        for (i = 0; i < self->count - self->cursor; i++) {
                if (self->in[i + self->cursor] == c) {
                        return i;
                }
                return -1;
        }

}


int buffer_dump(buffer_t * self) {
	
	int i;

	printf("[BUFFER: count %d\t cursor %d] \n", self->count, self->cursor);
	printf("[RAW] %d:", self->count);
	for (i = 0; i < self->count; i++) {
		printf("%02x ", self->in[i]);
	}
	printf("\n");

	printf("[LOGIC] %d:\n", self->count - self->cursor);
	for (i = 0; i < self->count - self->cursor; i++) {
		printf("%02x ", self->in[i + self->cursor]);
	}
	printf("\n");

}


int buffer_size(buffer_t * self) {

	return self->count - self->cursor;
}
