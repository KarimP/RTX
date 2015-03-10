#include "kcd.h"

void kcd_proc()
{
    MSG_BUF *msg = NULL;
    MSG_BUF *command_msg = NULL;
    char *buf = (char *)request_memory_block();
    int buf_size = 0;

    int sender_id = -1;
    int command_sender = -1;

    char *commands = (char *)request_memory_block();
    int *command_procs = (int *)request_memory_block();
    int num_commands = 0;

    int i = 0;
    int foundCommand = FALSE;

    while(TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);

        if (msg != NULL) {
            switch (msg->mtype) {
                case UART_INPUT:
                {
                    if (msg->mtext[0] == '%' && buf_size == 0) {
                        foundCommand = TRUE;
                    }

                    if (foundCommand && buf_size == 1) {
                        foundCommand = FALSE;
                        for (i = 0; i < num_commands; i++) {
                            if(commands[i] == msg->mtext[0]) {
                                foundCommand = TRUE;
                                command_sender = command_procs[i];
                                break;
                            }
                        }
                    }

                    if (foundCommand) {
                        buf[buf_size++] = msg->mtext[0];
                    } else {
                        buf_size = 0;
                        if (msg->mtext[0] == '%') {
                            buf[buf_size++] = msg->mtext[0];
                            foundCommand = TRUE;
                        }
                    }

                    if (foundCommand && (msg->mtext[0] == '\n' || msg->mtext[0] == '\r' || msg->mtext[0] == '%') && buf_size > 1) {

                        buf[--buf_size] = '\0';
                        command_msg = (MSG_BUF *)request_memory_block();
                        command_msg->mtype = DEFAULT;

                        for (i = 0; i < buf_size; ++i) command_msg->mtext[i] = buf[i];

                        send_message(command_sender, (void *)command_msg);

                        buf_size = 0;
                        if(msg->mtext[0] == '%') {
                            buf[buf_size++] = msg->mtext[0];
                        }
                    }

                    if (buf_size > MEM_BLK_SIZE) buf_size = 0;

                    msg->mtype = DEFAULT;
                    send_message(PID_CRT, (void *)msg);
                    break;
                }
                case KCD_REG:
                    if (msg->mtext[0] == '%') {
                        commands[num_commands] = msg->mtext[1];
                        command_procs[num_commands++] = sender_id;
                    }
                    release_memory_block((void *)msg);
                    break;
            }
        }
    }
}
