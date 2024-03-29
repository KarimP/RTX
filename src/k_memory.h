/**
 * @file:   k_memory.h
 * @brief:  kernel memory managment header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_MEM_H_
#define K_MEM_H_

#include "k_rtx.h"
#include "k_queue.h"
#include "k_process.h"
#include "k_process_queue.h"
#include "k_message.h"

/* ----- Definitions ----- */
#define RAM_END_ADDR 0x10008000

typedef queue* mem_q;
typedef queue_node* mem_blk;

/* ----- Variables ----- */
/* This symbol is defined in the scatter file (see RVCT Linker User Guide) */
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

/* ----- Functions ------ */
void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
void *k_non_blocking_request_memory_block(void);
int k_release_memory_block(void *);
int k_non_blocking_release_memory_block(void *);
int setup_heap(void);

#endif /* ! K_MEM_H_ */
