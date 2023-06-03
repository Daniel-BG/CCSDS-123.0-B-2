#include "queue.h"
#include "debug.h"

Queue* create_queue() {
    Queue* queue = calloc(1, sizeof(Queue));
    queue->capacity = 2;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    queue->array = calloc(queue->capacity, sizeof(char));
    return queue;
}

void free_queue(Queue * queue) {
	queue->capacity = 0;
	queue->size = 0;
	queue->front = 0;
	queue->rear = -1;
	free(queue->array);
}

void reverse_queue(Queue * queue) {
    unsigned char* new_array = calloc(queue->capacity, sizeof(char));

    int i, j;
    for (i = 0, j = queue->front; i < queue->size; i++, j = (j + 1) % queue->capacity) {
        new_array[queue->size - 1 - i] = queue->array[j];
    }

    queue->front = 0;
    queue->rear = queue->size - 1;

    free(queue->array);
    queue->array = new_array;
}

void apply_transform(Queue * queue, unsigned char (*transformer)(unsigned char)) {
    int i, j;
    for (i = 0, j = queue->front; i < queue->size; i++, j = (j + 1) % queue->capacity) {
        queue->array[j] = transformer(queue->array[j]);
    }
}

int is_full(Queue* queue) {
    return queue->size == queue->capacity;
}

int is_empty(Queue* queue) {
    return queue->size == 0;
}

void resize(Queue* queue, int new_capacity) {
    unsigned char* new_array = calloc(new_capacity, sizeof(char));

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

void enqueue(Queue* queue, unsigned char data) {
    if (is_full(queue)) {
        resize(queue, queue->capacity * 2);
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = data;
    queue->size++;
}

void enqueue_front(Queue* queue, unsigned char data) {
    if (is_full(queue)) {
        resize(queue, queue->capacity * 2);
    }

    queue->front = (queue->front - 1) % queue->capacity;
    queue->array[queue->front] = data;
    queue->size++;
}

int dequeue(Queue* queue) {
	if (is_empty(queue))
        DBG_EXIT

    char data = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    // Shrink the array if it is a quarter full
    if (queue->size <= queue->capacity / 4 && queue->capacity > 2) {
        resize(queue, queue->capacity / 2);
    }

    return data;
}

int dequeue_rear(Queue * queue) {
    if (is_empty(queue))
        DBG_EXIT

    char data = queue->array[queue->rear];
    queue->rear = (queue->rear - 1) % queue->capacity;
    queue->size--;

    // Shrink the array if it is a quarter full
    if (queue->size <= queue->capacity / 4 && queue->capacity > 2) {
        resize(queue, queue->capacity / 2);
    }

    return data;
}

unsigned char get_at(Queue* queue, int pos) {
    return queue->array[pos % queue->capacity];
}