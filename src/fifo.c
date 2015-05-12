
#include <stdlib.h>
#include <assert.h>


typedef struct {
	void * prev;
	void * next;
	void * obj;
} fifo_t;


typedef struct {
	void * first;
	void * last;
	int count;
} fifo_head_t;



void * fifo_new(void) {

	fifo_head_t * head = (fifo_head_t *) calloc(sizeof(fifo_t), 1);

	head->first = NULL;
	head->last = NULL;
	head->count = 0;

	return head;
}


void fifo_add(void * _self, void * o) {

	fifo_head_t * head = (fifo_head_t *) _self;
	fifo_t * first, * new_first;	

	new_first = (fifo_t *) calloc(sizeof(fifo_t), 1);
	new_first->obj = o;

	if ( head->first ) {
		
		/* We already have an enqeued object */
		first = head->first;
		new_first->next = (fifo_t *) first;
		first->prev = (fifo_t *) new_first;

	} else {

		/* This object is added to an empty fifo */
		head->last = new_first;
	}

	/* The new object is always placed first */
	head->first = new_first;
	head->count++;
}


void * fifo_get(void * _self) {
	
	fifo_head_t * head = (fifo_head_t *) _self;
	fifo_t * last = head->last;
	fifo_t * new_last;
	void * obj;
	
	if ( last->prev ) {

		/* We have an enqued object before last */
		new_last = last->prev;
		head->last = new_last;

	} else {
		
		/* We took the last object from the fifo; reset head */
		head->first = NULL;
		head->last = NULL;
	}

	obj = last->obj;
	free(last);

	head->count--;

	return obj;
}


int fifo_count(void * _self) {

	fifo_head_t * head = (fifo_head_t *) _self;
	return head->count;
}
