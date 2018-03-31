#include <stdio.h>
#include <stdlib.h>
#include "include/order.h"
#include "include/queue.h"
#include "include/types.h"

queue_o newQueue(){ 
	queue_o q = (queue_o) malloc(sizeof(queue_o));
	q->first = q->last = NULL;
	q->size = 0;
	return q;
}

void enQueue(queue_o q, order_o o){
	node_o * newNode = (node_o *) malloc(sizeof(node_o));
	newNode->order = o;

	if(q->first == NULL){
		q->first = newNode;
		q->last = newNode;
	} else { 
		q->last->next = newNode;
		q->last = newNode;
	}

	q->size++;
}

node_o * deQueue(queue_o q){
    if (q->first == NULL)
        return NULL;

    node_o * temp = q->first;
    q->first = q->first->next;
    
    if (q->first == NULL)
        q->last = NULL;

    q->size--;

    return temp;
}

boolean isEmpty(queue_o q){
	if(q-> first == NULL)
		return 1;
	else
		return 0;
}

unsigned int size(queue_o q){
	return q->size;
}