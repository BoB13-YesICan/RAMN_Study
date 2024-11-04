#include "all_headers.h"


void send_can_packet_showpayload(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id) {
    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (sendto(socket, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)addr, sizeof(*addr)) != sizeof(struct can_frame)) {
        perror("CAN packet send failed");
    } else {
        printf("CAN packet sent with ID: 0x%X, Data: ", can_id);
        for (int i = 0; i < length; i++) {
            printf("0x%02X ", data[i]);
        }
        printf("\n");
    }
}

void send_can_packet_sendonly(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id) {
    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (sendto(socket, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)addr, sizeof(*addr)) != sizeof(struct can_frame)) {
        perror("CAN packet send failed");
    } else {}
}

