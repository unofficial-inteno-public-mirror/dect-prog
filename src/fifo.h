
#ifndef FIFO_H
#define FIFO_H


void * fifo_new(void);
void fifo_add(void * _self, void * o);
void * fifo_get(void * self);
int fifo_count(void * _self);


#endif /* FIFO_H */
