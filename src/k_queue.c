#include "k_queue.h"

void initialize_queue(queue *q)
{
    q->first = NULL;
    q->last = NULL;
}

int enqueue(queue *q, queue_node *node)
{
    if (q->first == NULL) {
        q->first = node;
    } else {
        q->last->next = node;
    }

    q->last = node;
    node->next = NULL;

    return RTX_OK;
}

queue_node *dequeue(queue *q)
{
    queue_node *node = NULL;

    if (q == NULL || q->first == NULL) {
        return node;
    }

    node = q->first;
    q->first = node->next;

    if (q->first == NULL) {
        q->last = NULL;
    }

    return node;
}
