
#include <stdint.h>

enum states {
	BOOT_STATE,
	PRELOADER_STATE,
	FLASHLOADER_STATE,
};


struct state_handler {
	int state;
	void (*init_state)(int fd);
	void (*event_handler)(uint8_t *buf);
};


void state_add_handler(struct state_handler *s);
void * state_get_handler(void);



