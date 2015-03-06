/**
 * @file:   k_queue.h
 * @brief:
 * @date:   2014/01/17
 */

#ifndef K_QUEUE_H_
#define K_QUEUE_H_

#include "k_rtx.h"

typedef struct queue_node
{
    struct queue_node *next;
    //void * data //= next + 1 for mem, PCB for process
} queue_node;


typedef struct queue
{
    queue_node *first;
    queue_node *last;
} queue;

void initialize_queue(queue*);
int enqueue(queue*, queue_node*);
queue_node *dequeue(queue*);
int size(queue*);
int isEmpty(queue*);

#endif /* ! K_QUEUE_H_ */
