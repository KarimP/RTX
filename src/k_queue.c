#include "k_queue.h"

process_queue **ready_queue;
process_queue **blocked_queue;

void initialize_priority_queue(process_queue **priority_queue)
{
	int i;
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		priority_queue[i]->first = NULL;
		priority_queue[i]->last = NULL;
	}
}

int enqueue_priority_queue(process_queue **p_queue, PCB *item, int priority)
{
	if ((priority >= NUM_PRIORITIES) || (item == NULL)) {
		return RTX_ERR;
	}
	
	if (p_queue[priority]->first == NULL) {
		p_queue[priority]->first = item;
	} else {
		p_queue[priority]->last->mp_next = item;
	}
	
	p_queue[priority]->last = item;
	return RTX_OK;
}

PCB *dequeue_priority_queue(process_queue **p_queue, int priority)
{
	PCB *item;
	
	if (p_queue[priority] == NULL || p_queue[priority]->first == NULL) {
		return NULL;
	}
	
	item = p_queue[priority]->first;
	p_queue[priority]->first = item->mp_next;
		
	if (p_queue[priority]->first == NULL) {
		p_queue[priority]->last = NULL;
	}
	
	return item;
}
