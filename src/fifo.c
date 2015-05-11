
#include <stdlib.h>

#include "fifo.h"


static fifo_t * get_last(fifo_t * self) {
	
	fifo_t  * curr = self->next;

	while (curr && curr->next) {

		curr = curr->next;
	}

	return curr;
}


void * fifo_new(void) {

	fifo_t * head = (fifo_t *) calloc(sizeof(fifo_t), 1);

	return head;
}


void fifo_add(fifo_t * self, void * o) {
	
	fifo_t * first = self->next;
	fifo_t * new_first = (fifo_t *) calloc(sizeof(fifo_t), 1);

	new_first->obj = o;
	new_first->next = first;
	self->next = new_first;
}


void * fifo_get(fifo_t * self) {

	fifo_t * last = get_last(self);
	void * obj = last->obj;
	fifo_t * new_last = last->prev;

	free(last->next);

	return obj;
}

