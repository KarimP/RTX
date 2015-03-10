#include "kcd.h"

void kcd_proc()
{
    MSG_BUF *msg = NULL;
    MSG_BUF *command_msg = NULL;
    char *buf = (char *)request_memory_block();
    int buf_size = 0;    

    int sender_id = -1;

    MSG_BUF **commands = (MSG_BUF**)request_memory_block();
    int *command_procs = (int *)request_memory_block();
    int num_commands = 0;

    int i = 0, j = 0;
    int found_command = FALSE;
    int command_exists = FALSE;

    char *cur_cmd_buf = NULL;
    char *cmd_buf = NULL;

    while(TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);

        if (msg != NULL) {
            switch (msg->mtype) {
                case UART_INPUT:
                {
                    if (msg->mtext[0] == '%') found_command = TRUE;

                    if (found_command) buf[buf_size++] = msg->mtext[0];

                    if (found_command && (msg->mtext[0] == '\n' || msg->mtext[0] == '\r' || msg->mtext[0] == '%') && buf_size > 1) {
                        buf[--buf_size] = '\0';

                        for (i = 0; i < num_commands; ++i) {
                            cmd_buf = buf;
                            cur_cmd_buf = commands[i]->mtext;

                            while (*cmd_buf != '\0' && *cur_cmd_buf != '\0') {
                                if (*cmd_buf != *cur_cmd_buf) break;
                                cmd_buf++;
                                cur_cmd_buf++;
                            }

                            if (*cur_cmd_buf == '\0') {

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
                    if (num_commands < MEM_BLK_SIZE && msg->mtext[0] == '%') {
                        command_exists = FALSE;
                         
                        for (i = 0; i < num_commands; ++i) {
                            cmd_buf = msg->mtext;
                            cur_cmd_buf = commands[i]->mtext;

                            while (*cmd_buf != '\0' && *cur_cmd_buf != '\0') {
                                if (*cmd_buf != *cur_cmd_buf) break;
                                cmd_buf++;
                                cur_cmd_buf++;
                            }

                            if (*cur_cmd_buf == '\0' && *cmd_buf == '\0' && sender_id == command_procs[i]) command_exists = TRUE;
                        }

                        if (!command_exists) {
                            commands[num_commands] = msg;
                            command_procs[num_commands++] = sender_id;
                        } else {
                            release_memory_block(msg);
                        }
                    }
                    break;
            }
        }
    }
}
