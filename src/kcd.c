#include "kcd.h"

void kcd_proc()
{
    MSG_BUF *msg = NULL;
    MSG_BUF *command_msg = NULL;
    char *buf = (char *)request_memory_block();
    int buf_size = 0;

    int sender_id = -1;

    char *commands = (char *)request_memory_block();
    int *command_procs = (int *)request_memory_block();
    int num_commands = 0;

    int i = 0, j = 0;
    int found_command = FALSE;
    int command_exists = FALSE;

    while(TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);

        if (msg != NULL) {
            switch (msg->mtype) {
                case UART_INPUT:
                {
                    if (msg->mtext[0] == '%' && buf_size == 0) {
                        found_command = TRUE;
                    }

                    if (found_command && buf_size == 1) {
                        found_command = FALSE;
                        for (i = 0; i < num_commands; i++) {
                            if(commands[i] == msg->mtext[0]) {
                                found_command = TRUE;
                                break;
                            }
                        }
                    }

                    if (found_command) {
                        buf[buf_size++] = msg->mtext[0];
                    } else {
                        buf_size = 0;
                        if (msg->mtext[0] == '%') {
                            buf[buf_size++] = msg->mtext[0];
                            found_command = TRUE;
                        }
                    }

                    if (found_command && (msg->mtext[0] == '\n' || msg->mtext[0] == '\r' || msg->mtext[0] == '%') && buf_size > 1) {

                        buf[--buf_size] = '\0';
                        for (i = 0; i < num_commands; ++i) {
                            if (commands[i] == buf[1]) {
                                command_msg = (MSG_BUF *)request_memory_block();
                                command_msg->mtype = DEFAULT;

                                for (j = 0; j <= buf_size; ++j) command_msg->mtext[j] = buf[j];
                                send_message(command_procs[i], command_msg);
                            }
                        }

                        buf_size = 0;

                        if (msg->mtext[0] == '%') {
                            buf[buf_size++] = msg->mtext[0];
                        } else {
                            found_command = FALSE;
                        }
                    }

                    if (buf_size > MEM_BLK_SIZE) buf_size = 0;

                    msg->mtype = DEFAULT;
                    send_message(PID_CRT, msg);
                    break;
                }
                case KCD_REG:
                    if (num_commands < MEM_BLK_SIZE) {
                        if (msg->mtext[0] == '%') {
                            command_exists = FALSE;
                            for (i = 0; i < num_commands; ++i) {
                                if (command_procs[i] == sender_id && commands[i] == msg->mtext[1]) {
                                    command_exists = TRUE;
                                }
                            }

                            if (!command_exists) {
                                commands[num_commands] = msg->mtext[1];
                                command_procs[num_commands++] = sender_id;
                            }                            
                        }
                    }
                    release_memory_block(msg);
                    break;
            }
        }
    }
}
