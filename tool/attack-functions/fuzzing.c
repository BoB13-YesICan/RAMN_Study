#include "all_headers.h"

#define END_CANID 0x7FF
#define SLEEP_TIME 50000

void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    struct can_frame rx_frame;
    struct timeval timeout = {1, 0}; // receive timeout set (1sec)

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set socket options error");
        return;
    }

    for(int current_canid = canid; current_canid <= END_CANID; current_canid++) {
        if (time_diff > 0) {
            usleep (time_diff * 10000);
        }
        send_can_packet_showpayload(socket, addr, fuzzing.payload1, fuzzing.payload1_len, current_canid);

        struct timespec start, current;
        clock_gettime(CLOCK_REALTIME, &start);
        do{
            int nbytes = recvfrom(socket, &rx_frame, sizeof(struct can_frame), 0, NULL, NULL);
            if (nbytes > 0) {
                if (rx_frame.can_id == current_canid + 0x008 &&
                rx_frame.can_dlc == fuzzing.payload2_len &&
                memcmp(rx_frame.data, fuzzing.payload2, fuzzing.payload2_len) == 0) {
                    printf("Response found from CAN ID: 0x%03X\n", current_canid);
                    printf("Press 'n' to continue...\n");
                    char ch;
                    do {
                        ch = getchar();
                    } while (ch != 'n');
                }
            }clock_gettime(CLOCK_REALTIME, &current);
        }while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
    }
}

void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    for (int tx_can_id = canid; tx_can_id <= END_CANID; tx_can_id++) {
        send_can_packet(socket, addr, fuzzing.payload3, fuzzing.payload3_len, tx_can_id);

        if (time_diff == 0) break;
        usleep(time_diff*1000);
    }
}

void fuzzing_random_payload(int socket, struct sockaddr_can *addr, int canid, int time_diff){

}