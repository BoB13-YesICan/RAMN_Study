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

// Structure to hold the last timestamp for each CAN ID
struct timespec last_timestamp[MAX_CAN_IDS] = {0};

// Function to calculate time difference in microseconds
long long time_diff_microseconds(struct timespec start, struct timespec end) {
    long long sec_diff = end.tv_sec - start.tv_sec;
    long long nsec_diff = end.tv_nsec - start.tv_nsec;
    return sec_diff * 1000000LL + nsec_diff / 1000LL;
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
    printf("Timestamp (s.microseconds) | CAN ID | DLC | Data | Time Diff (us)\n");

    while (1) {
        struct timespec recv_time;

        // Receive CAN frame and capture timestamp
        nbytes = recv(s, &frame, sizeof(struct can_frame), 0);
        if (nbytes < 0) {
            perror("Read");
            close(s);
            return 1;
        }

        // Get high-resolution timestamp
        clock_gettime(CLOCK_MONOTONIC, &recv_time);

        // Calculate time difference if we've seen this CAN ID before
        long long time_diff = -1;
        if (last_timestamp[frame.can_id].tv_sec != 0 || last_timestamp[frame.can_id].tv_nsec != 0) {
            time_diff = time_diff_microseconds(last_timestamp[frame.can_id], recv_time);
        }

        // Store the current timestamp as the last timestamp for this CAN ID
        last_timestamp[frame.can_id] = recv_time;

        // Print data with time difference
        printf("%lld.%06ld | 0x%03X | %d | ",
               (long long)recv_time.tv_sec,
               recv_time.tv_nsec / 1000,
               frame.can_id,
               frame.can_dlc);

        for (int i = 0; i < frame.can_dlc; i++) {
            printf("%02X ", frame.data[i]);
        }

        if (time_diff != -1) {
            printf("| %lld", time_diff);  // Print time difference in microseconds
        } else {
            printf("| First packet");
        }

        printf("\n");
    }

    close(s);
    return 0;
}