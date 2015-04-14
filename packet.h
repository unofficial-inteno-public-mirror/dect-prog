
#include <stdint.h>
#include "buffer.h"

#define PACKET_SIZE 500

typedef struct {
	int fd;
	uint32_t size;
	uint8_t data[PACKET_SIZE];
} packet_t;

int packet_get(packet_t *p, buffer_t *b);
void packet_dump(packet_t *p);
void packet_dispatch(packet_t *p);
