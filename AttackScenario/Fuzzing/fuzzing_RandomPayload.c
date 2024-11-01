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
#include <stdint.h> // Added for uint64_t

#define PAYLOAD_SIZE 8
#define SLEEP_TIME 50000 // 기존 SLEEP_TIME을 유지하거나 조정할 수 있습니다.
#define START_CANID 0x000
#define END_CANID 0xFFF
#define TRANSMIT_COUNT 100 // 1초 동안 10ms 간격으로 100번 전송
#define TRANSMIT_INTERVAL_US 10000 // 10ms = 10,000 마이크로초

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

    // Create a socket
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Set up the CAN interface
    strncpy(ifr.ifr_name, can_interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; // Ensure null-terminated
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

    uint64_t counter = 0; // Initialize 64-bit payload counter

    // Loop through CAN IDs to send incrementing payloads
    for (int can_id = START_CANID; can_id <= END_CANID; can_id++) {
        tx_frame.can_id = can_id;
        tx_frame.can_dlc = PAYLOAD_SIZE;

        // Assign 8-byte payload based on the 64-bit counter (Big Endian)
        tx_frame.data[0] = (counter >> 56) & 0xFF;
        tx_frame.data[1] = (counter >> 48) & 0xFF;
        tx_frame.data[2] = (counter >> 40) & 0xFF;
        tx_frame.data[3] = (counter >> 32) & 0xFF;
        tx_frame.data[4] = (counter >> 24) & 0xFF;
        tx_frame.data[5] = (counter >> 16) & 0xFF;
        tx_frame.data[6] = (counter >> 8) & 0xFF;
        tx_frame.data[7] = counter & 0xFF;

        // Inner loop: Send the same payload multiple times with 10ms interval
        for (int i = 0; i < TRANSMIT_COUNT; i++) {
            // Send the CAN frame
            if (write(sock, &tx_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                perror("Write error");
                close(sock);
                return 1;
            }

            // Optionally, print the transmission log every N sends to reduce verbosity
            if (i == 0 || i == TRANSMIT_COUNT - 1) { // 예: 처음과 마지막 전송 시만 출력
                printf("CAN ID: 0x%03x | Payload: %02X %02X %02X %02X %02X %02X %02X %02X | Send %d/%d\n",
                       can_id,
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

            // Sleep for 10ms
            usleep(TRANSMIT_INTERVAL_US);
        }

        // Increment the counter for the next payload
        counter++;

        // Optional: Handle responses after transmitting the payload multiple times
        // 이 부분은 필요에 따라 조정할 수 있습니다.
        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);
        do {
            int nbytes = read(sock, &rx_frame, sizeof(struct can_frame));
            if (nbytes > 0) {
                // Check if the response CAN ID matches (sent CAN ID + 0x008)
                if (rx_frame.can_id == (can_id + 0x008) &&
                    rx_frame.can_dlc == PAYLOAD_SIZE &&
                    rx_frame.data[0] == 0x02 &&
                    rx_frame.data[1] == 0x7e &&
                    rx_frame.data[2] == 0x00) {
                    printf("Response found from CAN ID: 0x%03X\n", can_id + 0x008);
                    printf("Press 'n' to continue...\n");
                    char ch;
                    do {
                        ch = getchar();
                    } while (ch != 'n' && ch != EOF);
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &current);
        } while (((current.tv_sec - start.tv_sec) * 1000000 +
                  (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
    }

    // Close the socket
    close(sock);
    return 0;
}
