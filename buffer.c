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


int buffer_add(buffer_t * self, uint8_t *input, int count) {

	/* Only add data if we have enough room in the buffer */
	if ( self->count + count > self->max) {
		return -1;
	}

	memcpy(self->in + self->count, input, count);
	self->count += count;
	
	return 0;
}


int buffer_read(buffer_t * self, uint8_t *buf, int count) {

	/* Only add data if we have enough room in the buffer */
	if ( self->count + count > self->max) {
		return -1;
	}

	memcpy(buf, self->in + self->count, count);
	
	return 0;
}

int buffer_find(buffer_t * self, uint8_t c) {
	
	int i;

	/* Do we have a start of frame? */
        for (i = 0; i < self->count; i++) {
                if (self->in[i] == c) {
                        return i;
                }
                return -1;
        }

}


int buffer_dump(buffer_t * self) {
	
	int i;

	printf("[BUFFER: %d] \n", self->count);
	for (i = 0; i < self->count; i++) {
		printf("%02x ", self->in[i]);
	}
	printf("\n");
}
