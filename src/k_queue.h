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
    //void * data //= next + 1 for mem, PCB for process
} queue_node;


typedef struct queue
{
    queue_node *first;
    queue_node *last;
} queue;

/* queue methods */
void init_queue(queue*);
int enqueue(queue*, queue_node*);
int reverse_enqueue(queue*, queue_node*);
queue_node *dequeue(queue*);
queue_node *reverse_dequeue(queue*);
queue_node *remove_node(queue*, int (*compar)(const queue_node*, const void *), void*);
void rotate_queue(queue*);

/* priority queue methods */
void init_priority_queue(queue**);
int highest_filled_priority(queue**);
queue_node *get_highest_priority_node(queue**);
queue_node *dequeue_highest_priority_node(queue**);

#endif /* ! K_QUEUE_H_ */
