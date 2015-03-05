/**
 * @file:   usr_proc.h
 * @brief:  Two user processes header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_USR_PROC_H_
#define K_USR_PROC_H

#include "k_rtx.h"

/* System Processes */
void null_proc(void);
void k_set_procs(void);

/* initialization table item */
extern PROC_INIT g_proc_table[NUM_PROCS];

#endif /* K_USR_PROC_H_ */
