#ifndef QUEUE_H
#define QUEUE_H

#include "order.h"
#include "types.h"

typedef struct orderNode node_o;

typedef struct orderNode {
	order_o order;
	node_o * next;
} node_o;

typedef struct orderQueue * queue_o;

struct orderQueue {	
	node_o * first;
	node_o * last;
	int size;
};

/* Creates a new queue. */
queue_o newQueue();

/* Adds a new element at the end of the queue. */
void enQueue(queue_o q, order_o o);

/* Removes and returns the first element of the queue. */
node_o * deQueue(queue_o q);

/* Returns if the queue is empty. */
boolean isEmpty(queue_o q);

/* Returns the size of the queue. */
unsigned int size(queue_o q);

/*Frees de queue*/
void freeQueue(queue_o q);

#endif