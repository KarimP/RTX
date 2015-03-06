/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include "k_rtx.h"
#include "k_process_queue.h"
#include "k_usr_proc.h"
#include "k_queue.h"
#include "k_message.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */
/* ----- Functions ----- */

void process_init(void);               /* initialize all procs in the system */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);           /* kernel release_process function */

void atomic(int);

int k_send_message(int, void*);
void *k_receive_message(int*);
PCB* get_pcb_from_pid(int);

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */

extern void *k_request_memory_block(void);
extern int k_release_memory_block(void *);

extern process_queue **ready_queue;
extern process_queue **blocked_queue;

//extern PROC_INIT g_proc_table[NUM_PROCS];

#endif /* ! K_PROCESS_H_ */
