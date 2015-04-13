
#include <stdint.h>


typedef struct {
	uint8_t * in;
	uint8_t * cursor;
	uint32_t count;
	uint32_t max;
} buffer_t;

buffer_t * buffer_new(int size);
int buffer_add(buffer_t * self, uint8_t *input, int count);
int buffer_dump(buffer_t * self);
