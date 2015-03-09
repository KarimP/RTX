#include "crt.h"
#include "printf.h"
#include <LPC17xx.h>

void crt_proc(void)
{
	int sender_id = -1;
    MSG_BUF *msg = NULL;
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
            send_message(PID_UART_IPROC, msg);
		}
    }
}
