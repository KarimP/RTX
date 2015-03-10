#include "irq.h"
#include "k_rtx.h"
#include "printf.h"
//#include "timer.h"
#include "k_message.h"
#include "k_process.h"
#include "k_memory.h"
#include <LPC17xx.h>

extern void atomic(int);

#define BUFF_SIZE 100
uint8_t input_buffer[BUFF_SIZE] = "";
int index = 0;
extern queue *delayed_queue;

void k_irq_handler(irq_type type){
	switch(type){
		case TIMER:
					//need to perform a context switch to the timer Iprocess
					break;
		case UART:
					//need to perform a context switch to the uart Iprocess
					break;
		default:
				break;
	}

	//k_release_processor();
}

void timer_irq_proc(void)
{
	msg_Node *msg = (msg_Node *)delayed_queue->first;

    while (msg != NULL) {
        if (--msg->expire == 0) {
            dequeue(delayed_queue);
            k_send_message_with_node(msg);
            msg = (msg_Node *)delayed_queue->first;
        } else {
            msg = msg->next;
        }
    }
}

void uart_irq_proc(char key)
{
	int sender_id = -1;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	MSG_BUF *msg = NULL;
	char *buf = '\0';
	int forward_to_kcd = TRUE;

	#ifdef DEBUG_1
	printf("\n uart iprocess running \n");
	#endif /* DEBUG_1 */

	#ifdef DEBUG_HOTKEYS
	switch (key) { // printing process list and memory blocks available still left
		case '!':
			print_ready_procs();
			forward_to_kcd = FALSE;
			break;
		case '@':
			print_mem_blocked_procs();
			forward_to_kcd = FALSE;
			break;
		case '#':
			print_receive_blocked_procs();
			forward_to_kcd = FALSE;
			break;
	}
	#endif

	if (forward_to_kcd && key) {
		msg = (MSG_BUF *) k_non_blocking_request_memory_block();
		if (msg) {
			msg->mtype = UART_INPUT;
			msg->mtext[0] = key;
			msg->mtext[1] = '\0';

			k_non_blocking_send_message(PID_KCD, msg);
		}
	}

	msg = (MSG_BUF *) k_non_blocking_receive_message(&sender_id);

	while (msg != NULL) {
		buf = msg->mtext;

		while (*buf != '\0') {
			pUart->THR = *(buf);

			if (*(buf++) == '\r') {
				pUart->THR = '\n';
			}
		}
		k_non_blocking_release_memory_block(msg);

		msg = (MSG_BUF *) k_non_blocking_receive_message(&sender_id);
	}
}

// void uart_irq_proc()
// {
// 	int sender_id = -1;
// 	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
// 	MSG_BUF *msg = NULL;
// 	char *buf = '\0';
// 	int forward_to_kcd = TRUE;

// 	#ifdef DEBUG_1
// 	printf("\n uart iprocess running \n");
// 	#endif /* DEBUG_1 */

// 	while (TRUE) {
// 		msg = (MSG_BUF *) k_receive_message(&sender_id);

// 		while (msg != NULL) {
// 			buf = msg->mtext;

// 			if (msg->mtype == UART_INPUT) {
// 				#ifdef DEBUG_HOTKEYS
// 				switch (buf[0]) { // printing process list and memory blocks available still left
// 					// case '\r':
// 					// 	enter = 1;
// 					// 	break;
// 					case '!':
// 						print_ready_procs();
// 						forward_to_kcd = FALSE;
// 						break;
// 					case '@':
// 						print_mem_blocked_procs();
// 						forward_to_kcd = FALSE;
// 						break;
// 					case '#':
// 						print_receive_blocked_procs();
// 						forward_to_kcd = FALSE;
// 						break;
// 				}
// 				#endif

// 				if (forward_to_kcd) {
// 					k_send_message(PID_KCD, msg);
// 				} else {
// 					k_release_memory_block(msg);
// 				}
// 			} else {
// 				while (*buf != '\0') {
// 					pUart->THR = *(buf++);
// 				}
// 				k_release_memory_block(msg);
// 			}

// 			msg = (MSG_BUF *) k_receive_message(&sender_id);
// 		}

// 		k_release_processor();
// 	}
// }

