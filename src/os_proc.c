/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: SE 350 G017
 * @date:   2015/02/03
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "os_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
extern PROC_INIT g_proc_table[NUM_PROCS];

#define KCD_PID 7
#define CRT_PID 8

void set_os_procs(void) {
    g_proc_table[0].m_pid = (U32)(0);
    g_proc_table[0].m_priority = 4;
    g_proc_table[0].m_stack_size = 0x100;
    g_proc_table[0].mpf_start_pc = &null_proc;

    g_proc_table[7].m_pid = (U32)(7);
    g_proc_table[7].m_priority = 0;
    g_proc_table[7].m_stack_size = 0x100;
    g_proc_table[7].mpf_start_pc = &kcd_proc;

    g_proc_table[8].m_pid = (U32)(8);
    g_proc_table[8].m_priority = 0;
    g_proc_table[8].m_stack_size = 0x100;
    g_proc_table[8].mpf_start_pc = &crt_proc;

    g_proc_table[9].m_pid = (U32)(9);
    g_proc_table[9].m_stack_size = 0x1000;
    g_proc_table[9].m_priority = 0;
    g_proc_table[9].mpf_start_pc = &uart_irq_proc;

    // g_proc_table[10].m_pid = (U32)(10);
    // g_proc_table[10].m_stack_size = 0x1000;
    // g_proc_table[10].m_priority = 0;
    // g_proc_table[10].mpf_start_pc = &uart_irq_proc;
}

/**
 * @brief: null process
 */
void null_proc(void)
{
    while (1) {
        k_release_processor();
    }
}

