/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: SE350 G017
 * @date:   2015/02/03
 */

#include "k_memory.h"
#include <stdint.h>

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

#define MEM_BLK_SIZE 128

#define NUM_BLOCKS 20

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
   				/* stack grows down. Fully decremental stack */

/**
 * @brief: Start of the linked list of the memory blocks
 */
mem_q heap_q;

/**
 * @brief: Start of the heap allocated for memory blocks
 */
U8 *p_end;

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/
void memory_init(void)
{
	int i;

	p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;

	#ifdef DEBUG_1
	printf("pend = 0x%x \n\r", p_end);
	#endif /* ! DEBUG_1 */

	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB *);

	//allocate for PCBs
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB);
	}

	#ifdef DEBUG_1
	printf("gp_pcbs[0] = 0x%x \n\r", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n\r", gp_pcbs[1]);
	printf("gp_pcbs[2] = 0x%x \n\r", gp_pcbs[2]);
	printf("gp_pcbs[3] = 0x%x \n\r", gp_pcbs[3]);
	printf("gp_pcbs[4] = 0x%x \n\r", gp_pcbs[4]);
	printf("gp_pcbs[5] = 0x%x \n\r", gp_pcbs[5]);
	printf("gp_pcbs[6] = 0x%x \n\r", gp_pcbs[6]);
	#endif /* ! DEBUG_1 */

	/* prepare for alloc_stack() to allocate memory for stacks */
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack;
	}

	//allocate memory for ready queue
	ready_queue = (process_queue**)p_end;

	#ifdef DEBUG_1
	printf("Start of ready_queue array: 0x%x \n\r", ready_queue);
	#endif /* ! DEBUG_1 */

	p_end += (NUM_PRIORITIES) * sizeof(process_queue *);

	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		ready_queue[i] = (process_queue *)p_end;

		#ifdef DEBUG_1
		printf("ready_queue[%d]: 0x%x \n\r", i, ready_queue[i]);
		#endif /* ! DEBUG_1 */

		p_end += sizeof(process_queue);
	}
	initialize_priority_queue(ready_queue);

	//allocate memory for blocked queue
	blocked_queue = (process_queue**)p_end;

	#ifdef DEBUG_1
	printf("Start of blocked_queue array: 0x%x \n\r", blocked_queue);
	#endif /* ! DEBUG_1 */

	p_end += NUM_PRIORITIES * sizeof(process_queue*);

	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		blocked_queue[i] = (process_queue *)p_end;

		#ifdef DEBUG_1
		printf("blocked_queue[%d]: 0x%x \n\r", i, blocked_queue[i]);
		#endif /* ! DEBUG_1 */

		p_end += sizeof(process_queue);
	}
	initialize_priority_queue(blocked_queue);

}

/**
 * @brief allocates all of the remaining heap memory and sets up the memory queue
 * @return RTX_ERR on error and RTX_OK on success
 */
int setup_heap(void)
{
	mem_blk blk;
	int counter = 0;
	int i;

  	// allocate memory for heap memory queue
	heap_q = (queue*)p_end;
	p_end += sizeof(queue);//sizeof(mem_q) + sizeof(queue);

	initialize_queue(heap_q);
	//initialize memory linked list
	blk = (queue_node*)p_end;

	for (i = 0; i < NUM_BLOCKS; i++){
		enqueue(heap_q, blk);
		blk = (mem_blk)((char *)blk + MEM_BLK_SIZE + sizeof(queue_node));
		counter++;
	}

	#ifdef DEBUG_1
	printf("We have %d memory blocks\n\r", counter);
	printf("heap_q->first = 0x%x \n\r", heap_q->first);
	printf("heap_q->last = 0x%x \n\r", heap_q->last);
	printf("gpstack b= 0x%x \n\r", gp_stack);
	#endif /* ! DEBUG_1 */

	if (heap_q->first == (mem_blk)p_end){
		return RTX_OK;
	} else {
		return RTX_ERR;
	}
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b)
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */

	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);

	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack;
	}

	// #ifdef DEBUG_1
	//printf("gpstack = 0x%x \n\r", gp_stack);
	// #endif /* ! DEBUG_1 */

	return sp;
}

/**
 * @brief attempts to allocate a memory block for the process and blocks the process if that is not possible
 * @return returns a memory block when it is possible to do so
 */
void *k_request_memory_block(void)
{
	int process_priority;
	mem_blk blk = dequeue(heap_q);

	#ifdef DEBUG_1
	printf("k_request_memory_block: entering...\n\r");
	#endif /* ! DEBUG_1 */

	while (blk == NULL ) {

		process_priority = k_get_process_priority(gp_current_process->m_pid);
		pop_queue(ready_queue, gp_current_process->m_pid, process_priority);

		enqueue_priority_queue(blocked_queue, gp_current_process, process_priority);

		//set process state to BLOCKED_ON_RESOURCE ;
		gp_current_process->m_state = BLOCKED_ON_RESOURCE;

		k_release_processor();
	}

	#ifdef DEBUG_1
	printf("k_request_memory_block: exiting...\n\rblk requested is: 0x%x \nReturned blk is: 0x%x \nheap_q->first is: 0x%x \n\r\n\r", blk, blk + 1, heap_q->first);
	#endif /* ! DEBUG_1 */

	return blk + 1;
}

/**
 * @brief deallocates the passed in memory block if it is possible to do so
 * @return RTX_ERR on error and RTX_OK on success
 */
int k_release_memory_block(void *p_mem_blk)
{
	mem_blk rel_blk = (mem_blk)p_mem_blk - 1;
	int i;
	PCB *pcb = NULL;

	#ifdef DEBUG_1
	printf("k_release_memory_block: releasing block @ 0x%x \n\r", p_mem_blk);
	#endif /* ! DEBUG_1 */

	int a = (rel_blk == NULL);
	int b = ((U8 *)rel_blk < p_end);
	int c = ((U8 *)rel_blk + (MEM_BLK_SIZE + sizeof(queue_node)) > (U8*)gp_stack);
	int d = ((int)((U8 *)rel_blk - p_end)%(MEM_BLK_SIZE + sizeof(queue_node)) != 0);
	if ( a || b || c || d ) {
		return RTX_ERR;
	}

	if (enqueue(heap_q, rel_blk) != RTX_OK) {
		return RTX_ERR;
	}

	// removed highest priority blocked process that requested memory previously
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		pcb = dequeue_priority_queue(blocked_queue, i);
		if (pcb != NULL) {
			pcb->m_state = RDY;
			enqueue_priority_queue(ready_queue, pcb, i);
			break;
		}
	}

	#ifdef DEBUG_1
	printf("k_release_memory_block: exiting...\n\rheap_q->first is: 0x%x \nheap_q->first->next is: 0x%x \n\r\n\r", heap_q->first, heap_q->first->next);
	#endif /* ! DEBUG_1 */

	return RTX_OK;
}
