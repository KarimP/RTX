#include "set_priority.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

void set_priority(void)
{
    int sender_id = -1;
    MSG_BUF *msg = NULL;
    int pid_digit_1 = -1;
    int pid_digit_2 = -1;
    int pid = -1;
    int priority = -1;
    int valid = TRUE;
    int i;

    msg = (MSG_BUF *)request_memory_block();
    msg->mtype = KCD_REG;
    msg->mtext[0] = '%';
    msg->mtext[1] = 'C';
    msg->mtext[2] = '\0';

    send_message(PID_KCD, msg);

    while (TRUE)
    {
        msg = (MSG_BUF *) receive_message(&sender_id);
        valid = TRUE;

        if (sender_id == PID_KCD && msg->mtext[0] == '%' && msg->mtext[1] == 'C' &&  msg->mtext[2] == ' ') {
            i = 3;

            pid_digit_1 = msg->mtext[i++] - '0';
            if (msg->mtext[i] != ' ') {
                pid_digit_2 = msg->mtext[i++] - '0';
            }

            if (msg->mtext[i++] == ' ') {
                priority = msg->mtext[i++] - '0';
            } else {
                valid = FALSE;
            }

            if (msg->mtext[i] != ' ' && msg->mtext[i] != '\0') valid = FALSE;

            if (priority < HIGH && priority > LOWEST) {
                valid = FALSE;
            }

            if (pid_digit_2 != -1) {
                if ((pid_digit_1 != 1) || (pid_digit_2 < 0 || pid_digit_2 > 3)) valid = FALSE;
                else pid = 10 + pid_digit_2;
            } else {
                if (pid_digit_1 < 1 || pid_digit_1 > 9) valid = FALSE;
                else pid = pid_digit_1;
            }

        } else if (sender_id == PID_KCD) {
            valid = FALSE;
        }

        release_memory_block(msg);

        if (valid && sender_id == PID_KCD) {
            set_process_priority(pid, priority);
        } else if (sender_id == PID_KCD) {
            msg = (MSG_BUF *)request_memory_block();
            msg->mtype = DEFAULT;
            msg->mtext[0] = 'I';
            msg->mtext[1] = 'n';
            msg->mtext[2] = 'v';
            msg->mtext[3] = 'a';
            msg->mtext[4] = 'l';
            msg->mtext[5] = 'i';
            msg->mtext[6] = 'd';
            msg->mtext[7] = ' ';
            msg->mtext[8] = 'i';
            msg->mtext[9] = 'n';
            msg->mtext[10] = 'p';
            msg->mtext[11] = 'u';
            msg->mtext[12] = 't';
            msg->mtext[13] = '!';
            msg->mtext[14] = '\0';
            send_message(PID_CRT, msg);
        }
    }
}
