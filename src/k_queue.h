/**
 * @file:   k_queue.h
 * @brief:
 * @date:   2014/01/17
 */

#ifndef K_QUEUE_H_
#define K_QUEUE_H_

typedef struct queue_node
{
    struct queue_node *next;
} queue_node;


typedef struct queue
{
    queue_node *first;
    queue_node *last;
} queue;

void initialize_queue(queue*);
int enqueue(queue*, queue_node*);
queue_node *dequeue(queue*);

#endif /* ! K_QUEUE_H_ */
