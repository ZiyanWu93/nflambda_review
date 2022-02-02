#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "rte_mbuf.h"
// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    struct rte_mbuf **array;
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (struct rte_mbuf **)malloc(
        queue->capacity * sizeof(struct rte_mbuf *));
    return queue;
}

static inline void destroyQueue(struct Queue *queue)
{
    free(queue);
}

// Queue is full when size becomes
// equal to the capacity
static inline int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
static inline int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
static inline void enqueue(struct Queue *queue, struct rte_mbuf *item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
static inline struct rte_mbuf *dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    struct rte_mbuf *item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
static inline struct rte_mbuf *front(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->front];
}

// Function to get rear of queue
static inline struct rte_mbuf *rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->rear];
}

#endif /* COMMON_QUEUE */
