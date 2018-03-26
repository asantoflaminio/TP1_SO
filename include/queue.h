#ifndef QUEUE_H
#define QUEUE_H

#include "order.h"
#include "types.h"

typedef struct orderNode node_o;

struct orderNode {
	order_o order;
	node_o * next;
};

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

/* Removes the first element of the queue. */
node_o * deQueue(queue_o q);

/* Returns if the queue is empty. */
boolean isEmpty(queue_o q);

/* Returns the size of the queue. */
unsigned int size(queue_o q);

#endif