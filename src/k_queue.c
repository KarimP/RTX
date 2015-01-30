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
	item->mp_next = NULL;
	return RTX_OK;
}

PCB *dequeue_priority_queue(process_queue **p_queue, int priority)
{
	PCB *item = NULL;

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

int search_and_change_process_priority(process_queue **queue, int process_id, int prev_priority, int priority)
{
	PCB *pcb = NULL;
	PCB *prev_pcb = NULL;

	for (pcb = queue[prev_priority]->first; pcb != NULL; pcb = pcb->mp_next) {
		if (pcb->m_pid == process_id) {
			if (prev_pcb == NULL) {
				dequeue_priority_queue(queue, prev_priority);
			} else {
				prev_pcb->mp_next = pcb->mp_next;
			}

			enqueue_priority_queue(queue, pcb, priority);
			return RTX_OK;
		}

		prev_pcb = pcb;
	}

	return RTX_ERR;
}

int set_process_priority(int process_id, int priority)
{
	int i;
	int prev_priority = -1;

	if (priority > LOWEST_PRIORITY || priority < HIGHEST_PRIORITY) {
		return RTX_ERR;
	}

	for (i = 0; i < NUM_TEST_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			prev_priority = g_proc_table[i].m_priority;
			g_proc_table[i].m_priority = priority;
		}
	}

	//didn't find process in process table
	if (prev_priority == -1) {
		return RTX_ERR;
	}

	if (search_and_change_process_priority(ready_queue, process_id, prev_priority, priority) == RTX_ERR) {
		search_and_change_process_priority(blocked_queue, process_id, prev_priority, priority);
	}

	return RTX_OK;

}

int get_process_priority(int process_id)
{
	int i;
	for (i = 0; i < NUM_TEST_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			return g_proc_table[i].m_priority;
		}
	}

	return RTX_ERR;
}
