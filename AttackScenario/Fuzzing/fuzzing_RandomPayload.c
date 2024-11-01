#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>
#include <stdint.h>

#define PAYLOAD_SIZE 8
#define TRANSMIT_COUNT 100 //10ms interval 100times send (1sec)
#define TRANSMIT_INTERVAL_US 10000 // 10ms = 10,000 microseconds
#define START_PAYLOAD 0x03FFFFFFFFFFFFE0ULL // 64-bit initial payload

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <can_interface> <can_id>\n", argv[0]);
        return 1;
    }

    const char *can_interface = argv[1];
    unsigned int can_id_input;
    
    if (sscanf(argv[2], "%x", &can_id_input) != 1) {
        fprintf(stderr, "Invalid CAN ID format. Use hexadecimal (e.g., 0x1A3) or decimal (e.g., 419).\n");
        return 1;
    }

    //chekc CANID boundary
    if (can_id_input > 0xFFF) {
        fprintf(stderr, "CAN ID must be between 0x000 and 0xFFF.\n");
        return 1;
    }

    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame tx_frame, rx_frame;
    struct timeval timeout = {1, 0};        //1sec timeout

    //create socket
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    //set can interface
    strncpy(ifr.ifr_name, can_interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL error");
        close(sock);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    //bind socket to can interface
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind error");
        close(sock);
        return 1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set socket options error");
        close(sock);
        return 1;
    }

    uint64_t counter = START_PAYLOAD; // 64bit payload counter reeset

    //send frame to can_id_input
    while (1) {
        tx_frame.can_id = can_id_input;
        tx_frame.can_dlc = PAYLOAD_SIZE;

        
        tx_frame.data[0] = (counter >> 56) & 0xFF;
        tx_frame.data[1] = (counter >> 48) & 0xFF;
        tx_frame.data[2] = (counter >> 40) & 0xFF;
        tx_frame.data[3] = (counter >> 32) & 0xFF;
        tx_frame.data[4] = (counter >> 24) & 0xFF;
        tx_frame.data[5] = (counter >> 16) & 0xFF;
        tx_frame.data[6] = (counter >> 8) & 0xFF;
        tx_frame.data[7] = counter & 0xFF;

        //internal loop: send same payload 100 times(10ms interval)
        for (int i = 0; i < TRANSMIT_COUNT; i++) {
            //send can frame
            if (write(sock, &tx_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                perror("Write error");
                close(sock);
                return 1;
            }

            //print logs: start and end
            if (i == 0 || i == TRANSMIT_COUNT - 1) {
                printf("CAN ID: 0x%03X | Payload: %02X %02X %02X %02X %02X %02X %02X %02X | Send %d/%d\n",
                       can_id_input,
                       tx_frame.data[0],
                       tx_frame.data[1],
                       tx_frame.data[2],
                       tx_frame.data[3],
                       tx_frame.data[4],
                       tx_frame.data[5],
                       tx_frame.data[6],
                       tx_frame.data[7],
                       i + 1,
                       TRANSMIT_COUNT);
            }

            usleep(TRANSMIT_INTERVAL_US);
        }

        counter++;


    }
    close(sock);
    return 0;
}
