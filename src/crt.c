#include "crt.h"
#include "printf.h"
#include <LPC17xx.h>
#include <system_LPC17xx.h>

#define FALSE 0
#define TRUE 1
#define UART_INPUT 2
#define CRT_PID 8

void crt_proc(void)
{
	int sender_id;
    MSG_BUF *msg = NULL;
    char *buf;
    LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
    int isTurnedOn = 0;

    while (TRUE)
    {
        msg = (MSG_BUF *) receive_message(&sender_id);
    
        if (msg->mtype == CRT_ON)
        {
            isTurnedOn = TRUE;
            buf = msg->mtext;
            while (*buf != '\0')
            {
                pUart->THR = *buf;
                buf++;
            }
        }
        else if (msg->mtype == CRT_DISPLAY)
        {
            if (isTurnedOn == TRUE)
            {
                buf = msg->mtext;
                while (*buf != '\0')
                {
                    pUart->THR = *buf;
                    buf++;
                }
            }
        }
        else if (msg->mtype == CRT_OFF)
        {
            isTurnedOn = FALSE;
        }
        else if (msg->mtype == CRT_REG)
        {
                buf = msg->mtext;
                while (*buf != '\0')
                {
                    pUart->THR = *buf;
                    buf++;
                }
        }
        release_memory_block(msg);
    }
}
