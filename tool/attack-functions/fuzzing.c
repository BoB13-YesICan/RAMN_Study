#include "all_headers.h"

/*================================================================
fuzzing.c
This function performs the fuzzing attack

functions:
- fuzzing_find_uds: find valid UDS CANIDs, works like port scanning
- fuzzing_random_canid: randomly selects a CANID, fixed payload
- fuzzing_random_payload: randomly selects a payload, fixed CANID

==================================================================*/

#define END_CANID 0x7FF
#define SLEEP_TIME 5000
#define START_PAYLOAD 0x03FFFFFFFFFFFFE0ULL // 64-bit initial payload
#define TRANSMIT_COUNT 100 //10ms interval 100times send (1sec)[fuzzing_random_payload]

// void fuzzing_find_uds_logic(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
//     struct can_frame rx_frame;
//     struct timeval timeout = {1, 0}; // receive timeout set (1sec)
//     struct timespec start, current;

//     if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
//         perror("Set socket options error");
//         return;
//     }

//     for(int current_canid = canid; current_canid <= END_CANID; current_canid++) {
//         if (time_diff > 0) {
//             usleep (time_diff * 1000);
//         }
//         send_can_packet_showpayload(socket, addr, fuzzing.payload1, fuzzing.payload1_len, current_canid);

//         clock_gettime(CLOCK_REALTIME, &start);
//         do {
//             int nbytes = recvfrom(socket, &rx_frame, sizeof(struct can_frame), 0, NULL, NULL);
//             if (nbytes > 0) {
//                 if (rx_frame.can_id == (current_canid + 0x008) &&
//                     rx_frame.can_dlc == fuzzing.payload2_len &&
//                     (memcmp(rx_frame.data, fuzzing.payload2, fuzzing.payload2_len) == 0)) {
//                     printf("Response found from CAN ID: 0x%03X\n", current_canid);
//                     printf("Press 'n' to continue...\n");
//                     char ch;
//                     do {
//                         ch = getchar();
//                     } while (ch != 'n');
//                 }
//             } else if (nbytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
//                 perror("Receive error");
//                 return;
//             }
//             clock_gettime(CLOCK_REALTIME, &current);
//         } while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
//     }
// }

void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    fd_set readfds;
    int ret;
    char buf[64];
    set_nonblocking(STDIN_FILENO);

    struct can_frame rx_frame;
    struct timeval rx_timeout = {1, 0}; // receive timeout set (1sec)
    struct timeval timeout = {0, 0};
    struct timespec start, current;
    int current_canid = (canid - 1);

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &rx_timeout, sizeof(rx_timeout)) < 0) {
        perror("Set socket options error");
        return;
    }

    do {
        current_canid ++;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        ret = select(STDIN_FILENO +1, &readfds, NULL, NULL, &timeout);

        if (time_diff == 0) break;
        if (ret == -1) {
            perror("select fail");
            break;
        } else if (ret == 0) {
            send_can_packet_showpayload(socket, addr, fuzzing.payload1, fuzzing.payload1_len, current_canid);

            clock_gettime(CLOCK_REALTIME, &start);
            do {
                int nbytes = recvfrom(socket, &rx_frame, sizeof(struct can_frame), 0, NULL, NULL);
                if (nbytes > 0) {
                    if (rx_frame.can_id == (current_canid + 0x008) &&
                        rx_frame.can_dlc == fuzzing.payload2_len &&
                        (memcmp(rx_frame.data, fuzzing.payload2, fuzzing.payload2_len) == 0)) {
                        printf("Response found from CAN ID: 0x%03X\n", current_canid);
                        printf("Press 'n' to continue...\n");
                        char ch;
                        do {
                            ch = getchar();
                        } while (ch != 'n');
                    }
                } else if (nbytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("Receive error");
                    return;
                }
                clock_gettime(CLOCK_REALTIME, &current);
            }  while (((current.tv_sec - start.tv_sec) * 1000000 + (current.tv_nsec - start.tv_nsec) / 1000) < SLEEP_TIME);
        } else {
            if(FD_ISSET(STDIN_FILENO, &readfds)) {
                memset(buf, 0, sizeof(buf));
                if (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
                    if(buf[0] == 'm') {
                        printf(RED_TEXT"====================='m' detected=======================\n");
                        printf(RESET_COLOR"\n");
                        break;
                    }
                }
            }
        }
        usleep (1000 * time_diff);
    } while (current_canid < END_CANID);
}


void fuzzing_random_canid_logic(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    for (int tx_can_id = canid; tx_can_id <= END_CANID; tx_can_id++) {
        send_can_packet(socket, addr, fuzzing.payload3, fuzzing.payload3_len, tx_can_id);

        if (time_diff == 0) break;
        usleep(time_diff*1000);
    }
}

void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    fd_set readfds;
    int ret;
    char buf[64];
    set_nonblocking(STDIN_FILENO);
    struct timeval timeout = {0, 0};
    int tx_can_id = canid;

    do {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        ret = select(STDIN_FILENO +1, &readfds, NULL, NULL, &timeout);

        if (ret == -1) {
            perror("select fail");
            break;
        } else if (ret == 0) {
            send_can_packet(socket, addr, fuzzing.payload3, fuzzing.payload3_len, tx_can_id);
            tx_can_id ++;
        } else {
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                memset(buf, 0, sizeof(buf));
                if (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
                    if (buf[0] == 'm') {
                        printf(RED_TEXT"====================='m' detected=======================\n");
                        printf(RESET_COLOR"\n");
                        break;
                    }
                }
            }
        }
        if (time_diff == 0) break;
        usleep (1000 * time_diff);
    } while(tx_can_id <= END_CANID);
}

void fuzzing_random_payload_logic(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    struct can_frame tx_frame;
    uint64_t counter = START_PAYLOAD;
    do {
        tx_frame.data[0] = (counter >> 56) & 0xFF;
        tx_frame.data[1] = (counter >> 48) & 0xFF;
        tx_frame.data[2] = (counter >> 40) & 0xFF;
        tx_frame.data[3] = (counter >> 32) & 0xFF;
        tx_frame.data[4] = (counter >> 24) & 0xFF;
        tx_frame.data[5] = (counter >> 16) & 0xFF;
        tx_frame.data[6] = (counter >> 8) & 0xFF;
        tx_frame.data[7] = counter & 0xFF;

        for (int i = 0; i < TRANSMIT_COUNT; i++) {
            if (i == 0) {
                printf("Sending start, [1/100] CAN ID: ");
                printf(CYAN_TEXT"0x%03X", canid);
                printf(RESET_COLOR", Payload: ");
                printf(CYAN_TEXT"0x%016llX\n", counter);
                printf(RESET_COLOR);
            }
            if (i == TRANSMIT_COUNT - 1) {
                printf("Sending stop, [100/100] CAN ID: ");
                printf(CYAN_TEXT "0x%03X", canid);
                printf(RESET_COLOR ", Payload: ");
                printf(CYAN_TEXT "0x%016llX", counter);
                printf(RESET_COLOR"\n");
            }
            send_can_packet(socket,addr, tx_frame.data, 8, canid);
        }

        counter++;
        if (time_diff == 0) break;
        usleep(time_diff*1000);
    }while (1);

}

void fuzzing_random_payload (int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    fd_set readfds;
    int ret;
    char buf [64];
    set_nonblocking(STDIN_FILENO);
    struct timeval timeout = {0, 0};
    struct can_frame tx_frame;
    uint64_t counter = START_PAYLOAD;

    // tx_frame.data[0] = (counter >> 56) & 0xFF;
    // tx_frame.data[1] = (counter >> 48) & 0xFF;
    // tx_frame.data[2] = (counter >> 40) & 0xFF;
    // tx_frame.data[3] = (counter >> 32) & 0xFF;
    // tx_frame.data[4] = (counter >> 24) & 0xFF;
    // tx_frame.data[5] = (counter >> 16) & 0xFF;
    // tx_frame.data[6] = (counter >> 8) & 0xFF;
    // tx_frame.data[7] = counter & 0xFF;

    do {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        tx_frame.data[0] = (counter >> 56) & 0xFF;
        tx_frame.data[1] = (counter >> 48) & 0xFF;
        tx_frame.data[2] = (counter >> 40) & 0xFF;
        tx_frame.data[3] = (counter >> 32) & 0xFF;
        tx_frame.data[4] = (counter >> 24) & 0xFF;
        tx_frame.data[5] = (counter >> 16) & 0xFF;
        tx_frame.data[6] = (counter >> 8) & 0xFF;
        tx_frame.data[7] = counter & 0xFF;
        
        if (ret == -1) {
            perror("select fail");
            break;
        } else if (ret == 0) {
            for (int i = 0; i < TRANSMIT_COUNT; i++) {
                if (i = 0) {
                    printf("Sending start, [1/100] CAN ID: ");
                    printf(CYAN_TEXT"0x%03X", canid);
                    printf(RESET_COLOR", Payload: ");
                    printf(CYAN_TEXT"0x%016llX\n", counter);
                    printf(RESET_COLOR);
                }
                if (i == TRANSMIT_COUNT - 1) {
                printf("Sending stop, [100/100] CAN ID: ");
                printf(CYAN_TEXT "0x%03X", canid);
                printf(RESET_COLOR ", Payload: ");
                printf(CYAN_TEXT "0x%016llX", counter);
                printf(RESET_COLOR"\n");
                }
                send_can_packet(socket, addr, tx_frame.data, 8, canid);
            }
            counter ++;
        } else {
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                memset(buf, 0, sizeof(buf));
                if (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {
                    if (buf[0] == 'm') {
                        printf(RED_TEXT"====================='m' detected=======================\n");
                        printf(RESET_COLOR"\n");
                        break;
                    }
                }
            }
        }
        if(time_diff == 0) break;
        usleep (1000 * time_diff);
    } while(1);
}