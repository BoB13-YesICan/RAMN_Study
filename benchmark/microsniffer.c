#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>

#define MAX_CAN_IDS 2048  // Maximum number of CAN IDs to track

// Structure to store information for each CAN ID
typedef struct {
    struct timespec last_timestamp;  // Timestamp of the last packet
    int packet_count;                // Total packet count for this CAN ID
    long long time_interval;         // Time interval in microseconds
} CANID_Info;

// Array to store CAN ID information
CANID_Info canid_info[MAX_CAN_IDS] = {0};

// Function to calculate time difference in microseconds
long long time_diff_microseconds(struct timespec start, struct timespec end) {
    long long sec_diff = end.tv_sec - start.tv_sec;
    long long nsec_diff = end.tv_nsec - start.tv_nsec;
    return sec_diff * 1000000LL + nsec_diff / 1000LL;
}

// Function to display the current CAN ID information
void display_canid_info() {
    // Clear the screen using ANSI escape code
    printf("\033[2J\033[H");  // Clear screen and move cursor to the top left

    printf("Packet Count | CAN ID | Time Interval (us)\n");
    printf("-----------------------------------------\n");

    // Iterate through CAN IDs and display only those with packet data
    for (int i = 0; i < MAX_CAN_IDS; i++) {
        if (canid_info[i].packet_count > 0) {
            printf("%d           | 0x%03X  | ", canid_info[i].packet_count, i);
            if (canid_info[i].packet_count > 1) {
                printf("%lld\n", canid_info[i].time_interval);
            } else {
                printf("First packet\n");
            }
        }
    }
    fflush(stdout);  // Ensure the output is printed immediately
}

int main(int argc, char **argv) {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    ssize_t nbytes;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <can_interface>\n", argv[0]);
        return 1;
    }

    const char *ifname = argv[1];

    // Open a CAN_RAW socket
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket");
        return 1;
    }

    // Specify the CAN interface
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL");
        close(s);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        close(s);
        return 1;
    }

    printf("Listening on %s...\n", ifname);

    while (1) {
        struct timespec recv_time;

        // Receive CAN frame and record the timestamp
        nbytes = recv(s, &frame, sizeof(struct can_frame), 0);
        if (nbytes < 0) {
            perror("Read");
            close(s);
            return 1;
        }

        // Get high-resolution timestamp
        clock_gettime(CLOCK_MONOTONIC, &recv_time);

        // Update information for the CAN ID
        int can_id = frame.can_id;
        long long time_diff = -1;

        if (canid_info[can_id].packet_count > 0) {
            // Calculate the time difference with the previous packet
            time_diff = time_diff_microseconds(canid_info[can_id].last_timestamp, recv_time);
            canid_info[can_id].time_interval = time_diff;
        } else {
            // First packet for this CAN ID
            canid_info[can_id].time_interval = -1;
        }

        // Update last timestamp and increase packet count for the CAN ID
        canid_info[can_id].last_timestamp = recv_time;
        canid_info[can_id].packet_count++;

        // Display the updated information for all CAN IDs
        display_canid_info();
    }

    close(s);
    return 0;
}
