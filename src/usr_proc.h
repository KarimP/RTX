/**
 * @file:   usr_proc.h
 * @brief:  Two user processes header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef USR_PROC_H_
#define USR_PROC_H

#include "rtx.h"
#include "uart_polling.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define NUM_TESTS 4
#define PROC_1_DELAY 50//10000

void set_test_procs(void);

//Test procs
void send_message_test(void);
void receive_message_test(void);
void memory_management_test(void);
void priority_test(void);
void preemption_check(void);
void blocked_resource_test(void);

#endif /* USR_PROC_H_ */
