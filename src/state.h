
#include <stdint.h>

enum states {
	BOOT_STATE,
	PRELOADER_STATE,
	FLASHLOADER_STATE,
	APP_STATE,
	NVS_STATE,
};


typedef struct event {
	int fd;
	uint8_t *in;
	int incount;
	uint8_t *out;
	int outcount;
} event_t;


struct state_handler {
	int state;
	void (*init_state)(int fd);
	void (*event_handler)(event_t *e);
};


void state_add_handler(struct state_handler *s, int fd);
void * state_get_handler(void);

extern struct state_handler * preloader_state;
extern struct state_handler * flashloader_state;

