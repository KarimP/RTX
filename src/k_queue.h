/**
 * @file:   p_queue.h
 * @brief:
 * @date:   2014/01/17
 */

#ifndef K_QUEUE_H_
#define K_QUEUE_H_

#define NUM_PRIORITIES 4

#include "k_rtx.h"

/*
typedef struct queue_node
{
	PCB *proc;  //processor pcb
	struct queue_node *next;
} queue_node;
*/

typedef struct process_queue
{
	PCB *first;
	PCB *last;
} process_queue;

extern PROC_INIT g_proc_table[NUM_TEST_PROCS];

void initialize_priority_queue(process_queue**);
int enqueue_priority_queue(process_queue**, PCB*, int);
PCB *dequeue_priority_queue(process_queue**, int);

int set_process_priority(int, int);
int get_process_priority(int);

#endif /* ! K_QUEUE_H_ */
