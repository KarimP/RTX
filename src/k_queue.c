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

PCB *find_process_in_queue(int process_id, process_queue **queue) {
	
	
	return NULL;
}

int set_process_priority(int process_id, int priority){
	if (priority > LOWEST_PRIORITY || priority < HIGH_PRIORITY) {
		return RTX_ERR;
	}
	
	process_queue **queue = NULL;
	PCB *pcb = find_process_in_queue(process_id, start_queue);
	
	if (proc == NULL) {
		proc = find_process_in_queue(process_id, blocked_queue);
		if (proc == NULL) {
			return RTX_ERR;
		}
		queue = blocked_queue;
	} else {
		queue = start_queue;
	}
	
			
	return RTX_OK;
	
}

int get_process_priority(int process_id)
{
	int i;
	
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		PCB *pcb = NULL;
		
		//search start_queue
		for (pcb = start_queue[i]->first; 
				pcb != NULL; 
				pcb = pcb->mp_next) {
			
			if (pcb->pid == process_id) {
				return i;
			}
		}
		
		//search blocked_queue
		for (pcb = blocked_queue[i]->first; 
				pcb != NULL; 
				pcb = pcb->mp_next) {
			
			if (pcb->pid == process_id) {
				return i;
			}
		}
	}	
	
	PCB *proc = find_process_in_queue(process_id, start_queue);
	if (proc == NULL) {
		proc = find_process_in_queue(process_id, blocked_queue);
		if (proc == NULL) {
			return RTX_ERR;
		}
	}
	
	return proc->pid;
}
