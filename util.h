#include <stdint.h>

void util_dump(unsigned char *buf, int size, char *start);
void util_write(void *data, int size, int fd);

struct bin_img {
  uint8_t *img;
  int size;
  uint8_t size_msb;
  uint8_t size_lsb;
  uint8_t checksum;
};

