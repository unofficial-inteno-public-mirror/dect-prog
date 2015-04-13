
#include <stdint.h>


typedef struct {
	uint8_t * in;
	uint32_t count;
	uint32_t max;
} buffer_t;

buffer_t * buf_new(int size);

