#include "all_headers.h"
#include <time.h>

#define SLEEP_TIME 50000
#define START_CANID 0x000
#define END_CANID 0xFFF

void fuzzing_find_uds(int socket, struct sockaddr_can *addr) {
    struct can_frame tx_frame, rx_frame;
    struct timespec start, current;

    // CAN ID 범위 내에서 payload1 전송
    for (int can_id = START_CANID; can_id <= END_CANID; can_id++) {
        tx_frame.can_id = can_id;
        tx_frame.can_dlc = sizeof(fuzzing.payload1);
        memcpy(tx_frame.data, fuzzing.payload1, sizeof(fuzzing.payload1));


        // CAN 프레임 전송
        if (write(socket, &tx_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("Write error");
            return;
        }
        printf("Sent CAN ID: 0x%03x\n", can_id);

        // 지정된 시간 동안 응답을 기다림
        clock_gettime(CLOCK_MONOTONIC, &start);
        do {
            if (read(socket, &rx_frame, sizeof(struct can_frame)) > 0) {
                // 응답이 CAN ID + 0x008인지, 그리고 payload2와 일치하는지 확인
                if (rx_frame.can_id == (can_id + 0x008) &&
                    rx_frame.can_dlc == sizeof(fuzzing.payload2) &&
                    memcmp(rx_frame.data, fuzzing.payload2, sizeof(fuzzing.payload2)) == 0) {
                    printf("Response found from CAN ID: 0x%03X\n", can_id);
                    printf("Press 'n' to continue...\n");
                    char ch;
                    do {
                        ch = getchar();
                    } while (ch != 'n');
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &current);
        } while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
    }
}