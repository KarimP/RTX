#include "crt.h"
#include "printf.h"
#include <LPC17xx.h>
#include "uart.h"

void crt_proc(void)
{
	int sender_id = -1;
    MSG_BUF *msg = NULL;
    LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;

    while (TRUE)
    {
        msg = (MSG_BUF *) receive_message(&sender_id);

		if (msg->mtype == DEFAULT) {
            send_message(PID_UART_IPROC, msg);

            //trigger uart interrupt so it sees the message
            pUart->IER ^= IER_THRE; // toggle the IER_THRE bit
		}
    }
}
