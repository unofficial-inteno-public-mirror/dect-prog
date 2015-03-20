#include <string.h>

#include "state.h"

struct state_handler current_state;

struct state_handler * s = &current_state;
int source;

void state_add_handler(struct state_handler *s, int fd) {
	
	memcpy((void *)&current_state, (void *)s, sizeof(struct state_handler));
	source = fd;
	
}


void * state_get_handler(void) {

	return s->event_handler;
}

void state_transition(int state) {
	
	s->init_state(source);
	return;
}

