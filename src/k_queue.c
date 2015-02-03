#include "k_queue.h"

/**
 * @brief sets the first and last of the queue to null
 * @return nothing to return
 */
void initialize_queue(queue *q)
{
    q->first = NULL;
    q->last = NULL;
}

/**
 * @brief places the passed in node onto the end of the queue
 * @return RTX_ERR on error and RTX_OK on success
 */
int enqueue(queue *q, queue_node *node)
{
    //node is already in queue - return error
    if (node == NULL || node->next != NULL || node == q->last) {
        return RTX_ERR;
    }

    if (q->first == NULL) {
        q->first = node;
    } else {
        q->last->next = node;
    }

    q->last = node;
    node->next = NULL;

    return RTX_OK;
}

/**
 * @brief removes the first node in the queue if possible
 * @return first node in the queue or null if nothing is in the queue
 */
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

    node->next = NULL;

    return node;
}
