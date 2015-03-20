#include <string.h>

#include "state.h"

struct state_handler current_state;

struct state_handler * s = &current_state;

extern int dect_fd;

void state_add_handler(struct state_handler *s) {
	
	memcpy((void *)&current_state, (void *)s, sizeof(struct state_handler));
	
}


void * state_get_handler(void) {

	return s->event_handler;
}

void state_transition(int state) {
	
	s->init_state(dect_fd);
	return;
}

