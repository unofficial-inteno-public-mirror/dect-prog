#include <stdlib.h>
#include <stdio.h>


typedef struct {
	void * prev;
	void * next;
	int fd;
} list_t;

typedef struct {
	void * first;
	void * last;
	int count;
} list_head_t;

void * list_new(void) {
	
	list_head_t * head = (list_head_t *) calloc(sizeof(list_head_t), 1);
	
	head->first = NULL;
	head->last = NULL;
	head->count = 0;
	
	return head;
}


/* Objects are added to the end of the list */
void list_add(void * _self, int fd) {
	
	list_head_t * head = (list_head_t *) _self;
	list_t * new_last, * last;

	new_last = (list_t *) calloc(sizeof(list_t), 1);
	new_last->fd = fd;

	if ( head->last ) {
		
		/* We already have an object in the list */
		last = head->last;
		last->next = new_last;
		new_last->prev = last;
		
	} else {
		
		/* This object is added to an empty list */
		head->first = new_last;
	}

	head->last = new_last;
	head->count++;
	
	return;
}



void list_delete(void * _self, int fd) {
	
	list_head_t * head = (list_head_t *) _self;
	int i;
	list_t * obj, * prev, * next;

	if ( head->count == 0 ) {
		return;
	}
	
	obj = head->first;
	
	/* Loop over all objects in list */
	for (;;) {

		if ( obj->fd == fd ) {
			/* We have found our object */

			if ( ! obj->prev && obj->next ) {

				/* Object is at start of list with object follwing */
				next = obj->next;
				next->prev = NULL;
				head->first = next;

			} else if ( obj->prev && obj->next ) {

				/* Object in middle of list */
				next = obj->next;
				prev = obj->prev;
				next->prev = prev;
				prev->next = next;

			} else if ( ! obj->next && obj->prev ) {

				/* Object is at end of list with an object before it */
				prev = obj->prev;
				prev->next = NULL;
				head->last = prev;

			} else if ( ! obj->prev && ! obj->next ) {

				/* Object is only object in the list */
				head->first = NULL;
				head->last = NULL;
			}

			free(obj);
			head->count--;
			break;
		}

		if ( obj->next ) {
			/* Next object in list */
			obj = obj->next;
		} else {
			/* End of list */

			break;
		}
	}
	
	return;
}


void list_each(void * _self, void (*fn) (int fd)) {
	
	list_head_t * head = (list_head_t *) _self;
	void (*callback)(int fd) = fn;
	list_t * obj;

	if ( head->count == 0 ) {
		return;
	}
	
	obj = head->first;

	/* Loop over all objects in list */
	for (;;) {
		callback(obj->fd);
		
		if ( obj->next ) {
			/* Next object in list */
			obj = obj->next;
		} else {
			/* End of list */
			break;
		}
	}
	
	return;
}
