#include "k_queue.h"
#include "k_rtx.h"

/**
 * @brief sets the first and last of the queue to null
 * @return nothing to return
 */
void init_queue(queue *q)
{
    q->first = q->last = NULL;
}

/**
 * @brief sets the first and last of each queue in the priority queue to null
 * @return nothing to return
 */
void init_priority_queue(queue** pq)
{
    int i;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        pq[i]->first = pq[i]->last = NULL;
    }
}

/**
 * @brief places the passed in node onto the end of the queue
 * @return RTX_ERR on error and RTX_OK on success
 */
int enqueue(queue *q, queue_node *node)
{
    //node is already in queue - return error
    if (node == NULL || node == q->last) {
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
 * @brief places the passed in node onto the beginning of the queue
 * @return RTX_ERR on error and RTX_OK on success
 */
int reverse_enqueue(queue *q, queue_node *node)
{
    queue_node *first_node = q->first;

    //node is already in queue - return error
    if (node == NULL || node == q->last) {
        return RTX_ERR;
    }

    q->first = node;
    node->next = first_node;

    if (q->last == NULL) {
        q->last = node;
    }

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

/**
 * @brief removes the last node in the queue if possible
 * @return last node in the queue or NULL if nothing is in the queue
 */
queue_node *reverse_dequeue(queue *q)
{
    queue_node *node = NULL;
    queue_node *last_node = q->last;

    if (q == NULL || q->last == NULL) {
        return node;
    }

    if (q->first == q->last) {
        q->first = NULL;
        q->last = NULL;
    } else {
        //find second last node
        for (node = q->first; node->next != q->last; node = node->next) {}
        q->last = node;
    }

    return last_node;
}

/**
 * @brief Rotates queue so that the first element is now the last and the second last is the first
 * @return nothing to return
 */
void rotate_queue(queue* q)
{
    if (q == NULL || q->first == NULL || q->last == NULL) {
        return;
    }

    q->last->next = q->first;
    q->last = q->first;

    q->first = q->first->next;
    q->last->next = NULL;
}

/**
 * @brief Removes specified queue_node from the queue
 * @param compar - a function pointer which will take in a queue_node* and a void *property value
 *               and return non-zero if queue_node's property has that value and 0 if it doesn't
 * @return Returns the queue_node* or NULL otherwise
 */
queue_node *remove_node(queue *q, int (*compar)(const queue_node*, const void *), void *property)
{
    queue_node *node = NULL, *prev_node = NULL;

    for (node = q->first; node != NULL; node = node->next) {
        if (compar(node, property)) {
            if (prev_node == NULL) { //node is first node
                dequeue(q);
            } else {
                prev_node->next = node->next;

                if (prev_node->next == NULL) {
                    q->last = prev_node;
                }
            }

            if (node->next == NULL) {
                q->last = prev_node;
            }

            node->next = NULL;
            return node;
        }

        prev_node = node;
    }

    return NULL;
}

int highest_filled_priority(queue** pq)
{
    int i;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        if (pq[i]->first != NULL) {
            return i;
        }
    }
    return i;
}

queue_node *get_highest_priority_node(queue** pq)
{
    int i;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        if (pq[i]->first != NULL) {
            return pq[i]->first;
        }
    }
    return NULL;
}

queue_node *dequeue_highest_priority_node(queue** pq)
{
    int i;
    for (i = 0; i < NUM_PRIORITIES; ++i) {
        if (pq[i]->first != NULL) {
            return dequeue(pq[i]);
        }
    }
    return NULL;
}
