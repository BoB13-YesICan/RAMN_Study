#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>니
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>

#define PAYLOAD_SIZE 3
#define SLEEP_TIME 50000
#define START_CANID 0x7A0
#define END_CANID 0x800
#define RX_CANID_OFFSET 0x008

#define MAX_CANIDS 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <can_interface>\n", argv[0]);
        return 1;
    }

    const char *can_interface = argv[1];
    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame tx_frame, rx_frame;
    struct timeval timeout = {1, 0}; // 1-second timeout for recv

    // Read UDS_CANID_List.txt to get the list of CAN IDs
    FILE *file = fopen("UDS_CANID_List.txt", "r");
    if (!file) {
        perror("Failed to open UDS_CANID_List.txt");
        return 1;
    }

    int uds_canids[MAX_CANIDS];
    int num_canids = 0;
    while (fscanf(file, "%x,", &uds_canids[num_canids]) == 1 && num_canids < MAX_CANIDS) {
        num_canids++;
    }
    fclose(file);

    // Create a socket
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Set up the CAN interface
    strcpy(ifr.ifr_name, can_interface);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL error");
        close(sock);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind error");
        close(sock);
        return 1;
    }

    // Set receive timeout
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set socket options error");
        close(sock);
        return 1;
    }

    int detected_canids[MAX_CANIDS];
    int detected_count = 0;

    // Loop through CAN IDs to send payload 0x02 0x3e 0x00
    for (int can_id = START_CANID; can_id <= END_CANID; can_id++) {
        tx_frame.can_id = can_id;
        tx_frame.can_dlc = PAYLOAD_SIZE;
        tx_frame.data[0] = 0x02;
        tx_frame.data[1] = 0x3e;
        tx_frame.data[2] = 0x00;

        // Send the CAN frame
        if (write(sock, &tx_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("Write error");
            close(sock);
            return 1;
        }
        printf("Sent CAN ID: 0x%03x\n", can_id);

        // Read the response for SLEEP_TIME duration
        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);
        do {
            if (read(sock, &rx_frame, sizeof(struct can_frame)) > 0) {
                // Check if the response CAN ID matches any CAN ID in the UDS_CANID_List.txt
                for (int i = 0; i < num_canids; i++) {
                    if (rx_frame.can_id == uds_canids[i] + RX_CANID_OFFSET &&
                        rx_frame.can_dlc == PAYLOAD_SIZE &&
                        rx_frame.data[0] == 0x02 &&
                        rx_frame.data[1] == 0x7e &&
                        rx_frame.data[2] == 0x00) {
                        printf("Response found from CAN ID: 0x%03X\n", rx_frame.can_id);
                        detected_canids[detected_count++] = rx_frame.can_id;
                        printf("Press 'n' to continue...\n");
                        char ch;
                        do {
                            ch = getchar();
                        } while (ch != 'n');
                    }
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &current);
        } while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
    }

    // Print the detected CAN IDs
    printf("\nDetected CAN IDs with response:\n");
    for (int i = 0; i < detected_count; i++) {
        printf("0x%03X\n", detected_canids[i]);
    }
    printf("Total detected CAN IDs: %d\n", detected_count);

    // Close the socket
    close(sock);
    return 0;
}