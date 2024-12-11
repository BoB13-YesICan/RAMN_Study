#include "all_headers.h"

/*================================================================
can_sender.c
This function sends CAN packets with or without payloads.

Parameters:
- socket: CAN socket
- addr: CAN address
- data: CAN payload data
- length: CAN payload length
- can_id: CAN ID
- show_payload: Whether to show payload or not
- interval: interval between CAN packets in milliseconds
- time_diff: time difference between CAN packets in milliseconds

funtions:
- send_can_packet_showpayload: shows the payload and CANID when sending
_ send_can_packet don't shows the payload when sending
==================================================================*/


void send_can_packet_showpayload(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id) {
    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (sendto(socket, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)addr, sizeof(*addr)) != sizeof(struct can_frame)) {
        perror("CAN packet send failed");
    } else {
        printf("CAN packet sent with ID: ");
        printf(CYAN_TEXT"0x%X", can_id);
        printf(RESET_COLOR" && Data: ");
        printf(CYAN_TEXT);
        for (int i = 0; i < length; i++) {
            printf("0x%02X ", data[i]);
        }
        printf(RESET_COLOR"\n");
    }
}

void send_can_packet(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id) {
    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (sendto(socket, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)addr, sizeof(*addr)) != sizeof(struct can_frame)) {
        perror("CAN packet send failed");
    } else {}
}

