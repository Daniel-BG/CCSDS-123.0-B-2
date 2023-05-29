#include "queue.h"

Queue* create_queue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = 2;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    queue->array = (char*)malloc(queue->capacity * sizeof(char));
    return queue;
}

int is_full(Queue* queue) {
    return queue->size == queue->capacity;
}

int is_empty(Queue* queue) {
    return queue->size == 0;
}

void resize(Queue* queue, int new_capacity) {
    char* new_array = (char*)malloc(new_capacity * sizeof(char));

    int i, j;
    for (i = 0, j = queue->front; i < queue->size; i++, j = (j + 1) % queue->capacity) {
        new_array[i] = queue->array[j];
    }

    queue->capacity = new_capacity;
    queue->front = 0;
    queue->rear = queue->size - 1;

    free(queue->array);
    queue->array = new_array;
}

void enqueue(Queue* queue, char data) {
    if (is_full(queue)) {
        resize(queue, queue->capacity * 2);
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = data;
    queue->size++;
}

int dequeue(Queue* queue) {
	if (is_empty(queue))
		return -1; //signal error

    char data = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    // Shrink the array if it is a quarter full
    if (queue->size <= queue->capacity / 4 && queue->capacity > 2) {
        resize(queue, queue->capacity / 2);
    }

    return data;
}