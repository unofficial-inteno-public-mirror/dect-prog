#include <stdlib.h>


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
	list_t * list_obj;
	printf("list_delete: %d\n", fd);
	printf("head->count: %d\n", head->count);
	if ( head->count == 0 ) {
		printf("barf\n");
		return;
	}
	
	list_obj = head->first;

	if ( list_obj->fd == fd ) {
		
		printf("delete fd: %d\n", list_obj->fd);

		head->first = NULL;
		head->last = NULL;
		free(list_obj);
		
		head->count--;
	}
	
	printf("head->count: %d\n", head->count);
}

