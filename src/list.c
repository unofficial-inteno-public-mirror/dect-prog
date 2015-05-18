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
	printf("list_add: %d\n", fd);

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
	printf("head->count: %d\n", head->count);
}



void list_delete(void * _self, int fd) {
	
	list_head_t * head = (list_head_t *) _self;
	int i;
	list_t * obj, * prev, * next;

	printf("list_delete: %d\n", fd);

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
				head->first = next;

			} else if ( obj->prev && obj->next ) {
				
				/* Object in middle of list */
				next = obj->next;
				prev = obj->prev;
				next->prev = prev;
				prev->next = next;

			} else if ( ! obj->next && obj->prev ) {

				/* Object is at end of list with an object before it */
				head->last = obj->prev;

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
	
	printf("head->count: %d\n", head->count);
}

