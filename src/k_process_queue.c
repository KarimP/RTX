#include "k_process_queue.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

process_queue **ready_queue;
process_queue **blocked_queue;

void printOutQueues() {
	#ifdef DEBUG_0
	int i;
	PCB *pcb;

	// printf("\n---------- Blocked Queue ---------- ");
	// for (i = 0; i < NUM_PRIORITIES; ++i) {
	// 	printf("\nblocked_queue[%d]: \n", i);
	// 	for (pcb = blocked_queue[i]->first; pcb != NULL; pcb=pcb->mp_next) {
	// 		printf("%d, ", pcb->m_pid);
	// 	}
	// }

	printf("\n\n---------- Ready Queue ----------");
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		printf("\nready_queue[%d]: \n", i);
		for (pcb = ready_queue[i]->first; pcb != NULL; pcb=pcb->mp_next) {
			printf("%d, ", pcb->m_pid);
		}

		printf(" LAST: %d, ", ready_queue[i]->last->m_pid);
	}

	// printf("\nready_queue[1]: \n");
	// for (pcb = ready_queue[1]->first; pcb != NULL; pcb=pcb->mp_next) {
	// 	printf("%d, ", pcb->m_pid);
	// }

	printf("\n");
	#endif /* DEBUG_1 */
}

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
	if ( (item->m_pid != 0 ) && ((priority > LOWEST_PRIORITY) || (priority < HIGHEST_PRIORITY) || (item == NULL))) {
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

	item->mp_next = NULL;

	return item;
}

PCB *pop_queue(process_queue **queue, int process_id, int priority) {
	PCB *pcb = NULL;
	PCB *prev_pcb = NULL;

	for (pcb = queue[priority]->first; pcb != NULL; pcb = pcb->mp_next) {
		if (pcb->m_pid == process_id) {
			if (prev_pcb == NULL) {
				dequeue_priority_queue(queue, priority);
			} else {
				prev_pcb->mp_next = pcb->mp_next;

				if (prev_pcb->mp_next == NULL) {
					queue[priority]->last = prev_pcb;
				}
			}

			if (pcb->mp_next == NULL) {
				queue[priority]->last = prev_pcb;
			}

			pcb->mp_next = NULL;
			return pcb;
		}

		prev_pcb = pcb;
	}

	return NULL;
}

//SHOULD CALL POP THEN ENQUEUE
PCB *search_and_change_process_priority(process_queue **queue, int process_id, int prev_priority, int priority)
{
	PCB *pcb = NULL;
	PCB *prev_pcb = NULL;

	for (pcb = queue[prev_priority]->first; pcb != NULL; pcb = pcb->mp_next) {
		if (pcb->m_pid == process_id) {
			if (prev_pcb == NULL) {
				dequeue_priority_queue(queue, prev_priority);
			} else {
				prev_pcb->mp_next = pcb->mp_next;

				if (prev_pcb->mp_next == NULL) {
					queue[prev_priority]->last = prev_pcb;
				}
			}

			pcb->mp_next = NULL;
			enqueue_priority_queue(queue, pcb, priority);
			return pcb;
		}

		prev_pcb = pcb;
	}

	return NULL;
}

int get_highest_queue_priority(process_queue **queue) {
	int i;
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		if (queue[i]->first != NULL) {
			return i;
		}
	}

	return i;
}

int k_set_process_priority(int process_id, int priority)
{
	int i;

	int prev_priority = -1;
	PCB *proc;
	int current_process_priority;
	process_queue **queue = ready_queue;

	if (process_id == 0 || priority > LOWEST_PRIORITY || priority < HIGHEST_PRIORITY) {
		return RTX_ERR;
	}

	for (i = 0; i < NUM_TEST_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			prev_priority = g_proc_table[i].m_priority;
			g_proc_table[i].m_priority = priority;
		}

		if (g_proc_table[i].m_pid == gp_current_process->m_pid) {
			current_process_priority = g_proc_table[i].m_priority;
		}
	}

	//didn't find process in process table
	if (prev_priority == -1) {
		return RTX_ERR;
	} else if (prev_priority == priority) { //do nothing

		return RTX_OK;
	}

	proc = search_and_change_process_priority(queue, process_id, prev_priority, priority);
	if (proc == NULL) {
		queue = blocked_queue;
		proc = search_and_change_process_priority(queue, process_id, prev_priority, priority);
	}

	if (current_process_priority > get_highest_queue_priority(ready_queue)) {
		k_release_processor();
	}

	return RTX_OK;

}

int k_get_process_priority(int process_id)
{
	int i;
	for (i = 0; i < NUM_TEST_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			return g_proc_table[i].m_priority;
		}
	}

	return RTX_ERR;
}
