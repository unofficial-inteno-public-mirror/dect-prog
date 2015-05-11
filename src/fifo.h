
#ifndef FIFO_H
#define FIFO_H

typedef struct {
	void * prev;
	void * next;
	void * obj;
} fifo_t;



void * fifo_new(void);
void fifo_add(fifo_t * self, void * o);
void * fifo_get(fifo_t * self);



#endif /* FIFO_H */
