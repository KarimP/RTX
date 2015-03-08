#include "crt.h"
#include "printf.h"
#include <LPC17xx.h>

void crt_proc(void)
{
	  int sender_id = -1;
    MSG_BUF *msg = NULL;
    char *buf = '\0';
    LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
    int isTurnedOn = FALSE;

    while (TRUE)
    {
        msg = (MSG_BUF *) receive_message(&sender_id);
			
				switch(msg->mtype) {
					case CRT_ON:
						isTurnedOn = TRUE;
						break;
					case CRT_OFF:
						isTurnedOn = FALSE;
						break;
				}
				
				if ((msg->mtype == CRT_DISPLAY && isTurnedOn) || msg->mtype == CRT_REG) {
					buf = msg->mtext;
					while (*buf != '\0') {
						pUart->THR = *buf;
						buf++;
					}
				}
				
        release_memory_block(msg);
    }
}
