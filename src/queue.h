#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdlib.h>



typedef struct {
    char * array;
    int size;
    int capacity;
    int front;
    int rear;
} Queue;

Queue* create_queue();
void free_queue(Queue * queue);
int is_full(Queue* queue);
int is_empty(Queue* queue);
void resize(Queue* queue, int new_capacity);
void enqueue(Queue* queue, char data);
int dequeue(Queue* queue);
char get_at(Queue* queue, int pos);

#endif // __QUEUE_H__