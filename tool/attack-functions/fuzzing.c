#include "all_headers.h"

#define SLEEP_TIME 50000
#define START_CANID 0x000
#define END_CANID 0xFFF
#define TRANSMIT_COUNT 100 //10ms interval 100times send (1sec)
//#define TRANSMIT_INTERVAL_US 10000 // 10ms = 10,000 microseconds
#define START_PAYLOAD 0x03FFFFFFFFFFFFE0ULL // 64-bit initial payload

void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
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

//=====================fuzzingrandompayload=====================

void fuzzing_randompayload(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
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

    int s = socket;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame tx_frame, rx_frame;
    struct timeval timeout = {1, 0};        //1sec timeout

    //create socket
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    //set can interface
    strncpy(ifr.ifr_name, can_interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL error");
        close(s);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;


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
