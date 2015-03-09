#include "k_process_queue.h"

extern PCB *gp_current_process;
extern int is_blocking;

process_queue **ready_queue;
process_queue **blocked_queue;

/**
 * @brief sets the first and last of each queue to null within the priority queue
 * @return nothing to return
 */
void initialize_priority_queue(process_queue **priority_queue)
{
	int i;
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		priority_queue[i]->first = NULL;
		priority_queue[i]->last = NULL;
	}
}

/**
 * @brief places the passed in node onto the end of the queue for the given priority
 * @return RTX_ERR on error and RTX_OK on success
 */
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

/**
 * @brief removes the first node in the queue for the given prioity if possible
 * @return first node in the queue or null if nothing is in the queue
 */
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

/**
 * @brief Removes specified PCB from the queue
 * @return Returns the PCB or NULL otherwise
 */
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

			pcb->mp_next = NULL;
			return pcb;
		}

		prev_pcb = pcb;
	}

	return NULL;
}

/**
 * @brief Searches through the prioity queue until it finds the PCB, then it removes it and puts it in the appropraite queue
 * @return Returns the PCB that was changed or NULL otherwise
 */
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

/**
 * @brief gets the highest prioity node within the prioity queue
 * @return highest priority within the priority queue
 */
int get_highest_queue_priority(process_queue **queue) {
	int i;
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		if (queue[i]->first != NULL) {
			return i;
		}
	}

	return i;
}

/**
 * @brief Sets the prioity of the process with a given Process ID
 * @return Returns RTX_ERR on error and RTX_OK otherwise
 */
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

	for (i = 0; i < NUM_PROCS; ++i) {
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

	if (current_process_priority > get_highest_queue_priority(ready_queue) && is_blocking) {
		k_release_processor();
	}

	return RTX_OK;

}

/**
 * @brief Gets the prioity for the given priority ID
 * @return Priority of a given process ID
 */
int k_get_process_priority(int process_id)
{
	int i;
	for (i = 0; i < NUM_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			return g_proc_table[i].m_priority;
		}
	}

	return RTX_ERR;
}
