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
#include "k_queue.h"
#include "k_message.h"
#include "k_memory.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */
/* ----- Functions ----- */

void process_init(void);               /* initialize all procs in the system */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);           /* kernel release_process function */

void atomic(int);

int k_send_message(int, void*);
int k_non_blocking_send_message(int, void*);
void *k_receive_message(int*);
void *k_non_blocking_receive_message(int*);
int k_delayed_send(int, void*, int);
int k_send_message_with_node(msg_Node*);
PCB* get_pcb_from_pid(int);
int k_release_processor_irq(int);

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */
extern void set_os_procs(void);

// UART Debug procs
void print_ready_procs(void);
void print_mem_blocked_procs(void);
void print_receive_blocked_procs(void);
void print_number_of_memory_blocks(void);
void print_list_of_processes(void);

#endif /* ! K_PROCESS_H_ */
