
#include <stdint.h>

#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
	uint8_t * buf_start;
	uint8_t * buf_end;
	uint8_t * data_start;
	uint8_t * data_end;
	uint32_t count;
	uint32_t buf_size;
} buffer_t;

buffer_t * buffer_new(int size);
int buffer_add(buffer_t * self, uint8_t *input, int count);
int buffer_dump(buffer_t * self);
int buffer_rewind(buffer_t * self, int count);

#endif /* BUFFER_H */
