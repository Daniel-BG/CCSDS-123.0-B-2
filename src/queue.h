#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdlib.h>



typedef struct {
    unsigned char * array;
    int size;
    int capacity;
    int front;
    int rear;
} Queue;

Queue* create_queue();
void free_queue(Queue * queue);
void reverse_queue(Queue * queue);
void apply_transform(Queue * queue, unsigned char (*transformer)(unsigned char));
int is_full(Queue* queue);
int is_empty(Queue* queue);
void resize(Queue* queue, int new_capacity);
void enqueue(Queue* queue, unsigned char data);
int dequeue(Queue* queue);
unsigned char get_at(Queue* queue, int pos);

#endif // __QUEUE_H__