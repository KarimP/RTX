/**
 * @file:   p_queue.h
 * @brief:
 * @date:   2014/01/17
 */

#ifndef K_PROCESS_QUEUE_H_
#define K_PROCESS_QUEUE_H_

#define NUM_PRIORITIES 5

#include "k_rtx.h"
#include "k_process.h"

typedef struct process_queue
{
	PCB *first;
	PCB *last;
} process_queue;

extern PROC_INIT g_proc_table[NUM_PROCS];

//METHODS
void initialize_priority_queue(process_queue**);
int enqueue_priority_queue(process_queue**, PCB*, int);
PCB *dequeue_priority_queue(process_queue**, int);
PCB *pop_queue(process_queue**, int, int);

int k_set_process_priority(int, int);
int k_get_process_priority(int);

#endif /* ! K_PROCESS_QUEUE_H_ */
