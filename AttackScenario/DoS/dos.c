#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *interface = argv[1];
    int canid = argv[2];
    int interval = atoi(argv[4]);

    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface);
    ioctl(s, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr = {0};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    struct can_frame frame;
    frame.can_id = canid;
    frame.can_dlc = 8;
    unsigned char payload1[8] = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78};
    memcpy(frame.data, payload1, 8);

    while (1) {
        write(s, &frame, sizeof(struct can_frame));
        usleep(interval);
    }

    close(s);
    return 0;
}
