#include "irq.h"
#include "k_rtx.h"
#include "k_message.h"
#include "k_process.h"
#include "k_memory.h"

extern void atomic(int);
extern queue *delayed_queue;

//called by timer once per millisecond
//decrements messages's expire time in delayed queue by one
//sends messages when they are expired 
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

//called bu uart_irq when an interrupt is caused
//forwards input to KCD
//outputs messages from CRT to display
//handles debug hotkeys
void uart_irq_proc(char key)
{
	int sender_id = -1;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	MSG_BUF *msg = NULL;
	char *buf = '\0';

	#ifdef DEBUG_1
	printf("\n uart iprocess running \n");
	#endif /* DEBUG_1 */

	#ifdef DEBUG_HOTKEYS
	switch (key) { // printing process list and memory blocks available still left
		case '!':
			print_ready_procs();
			break;
		// case '"':
		case '@':
			print_mem_blocked_procs();
			break;
		// case '/':
		case '#':
			print_receive_blocked_procs();
			break;
		case '$':
			print_number_of_memory_blocks();
			break;
		// case '?':
		case '^':
			print_list_of_processes();
			break;
	}
	#endif

	if (key) {
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
