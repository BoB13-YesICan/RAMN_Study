#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#define DEFAULT_INTERVAL_MS 10  // 기본 데이터 전송 간격 (10ms)
#define MAX_DATA_BYTES 8        // CAN 데이터 필드의 최대 크기

// 밀리초 단위의 딜레이 함수
void delay_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("사용법: %s [인터페이스명] [보낼 ECUID#데이터] -diff [시간간격(ms)]\n", argv[0]);
        printf("예시: %s can0 7E0#1102030405060708 -diff 10\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    time_t start_time, current_time;
    int last_printed_time = 0;
    int interval_ms = -1;  // -1로 초기화하여 -diff 옵션이 없는 경우를 구분

    // 명령행 인자 처리
    char *interface_name = argv[1];
    char *id_and_data = argv[2];
    char *data_str;

    // CAN ID와 데이터 분리
    char *delimiter = strchr(id_and_data, '#');
    if (delimiter == NULL) {
        printf("잘못된 데이터 형식입니다. CAN ID와 데이터는 '#'로 구분되어야 합니다.\n");
        return 1;
    }

    *delimiter = '\0'; // '#'을 null로 바꿔서 CAN ID와 데이터를 분리
    unsigned int ecu_id = (unsigned int)strtol(id_and_data, NULL, 16);  // CAN ID를 16진수로 변환
    data_str = delimiter + 1;  // 데이터 부분

    // 데이터 문자열 처리 (보낼 데이터 추출)
    int data_length = strlen(data_str);
    if (data_length <= 0 || data_length > MAX_DATA_BYTES * 2 || data_length % 2 != 0) {
        printf("잘못된 데이터 형식입니다. 데이터는 # 뒤에 최대 8바이트의 16진수 형식이어야 합니다.\n");
        return 1;
    }

    frame.can_dlc = data_length / 2;  // 데이터 길이 설정
    for (int i = 0; i < frame.can_dlc; i++) {
        sscanf(&data_str[i * 2], "%2hhx", &frame.data[i]);  // 16진수 데이터 파싱
    }

    // -diff 인자 처리 (있다면 간격 설정)
    if (argc > 4 && strcmp(argv[3], "-diff") == 0) {
        interval_ms = atoi(argv[4]);
    }

    // 소켓 생성
    if ((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket 생성 실패");
        return 1;
    }

    // 인터페이스 이름 설정
    strcpy(ifr.ifr_name, interface_name);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl 실패");
        close(sockfd);
        return 1;
    }

    // 주소 설정
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // 소켓을 CAN 인터페이스에 바인딩
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("소켓 바인딩 실패");
        close(sockfd);
        return 1;
    }

    // CAN 프레임 데이터 설정
    frame.can_id = ecu_id;  // CAN ID 설정

    // 시작 시간 기록
    start_time = time(NULL);

    // CAN 프레임 전송
    if (write(sockfd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("CAN 프레임 전송 실패");
        close(sockfd);
        return 1;
    }

    printf("패킷 전송 시작\n");

    // -diff 옵션이 있는 경우에만 반복적으로 전송
    if (interval_ms > 0) {
        while (1) {
            delay_ms(interval_ms);  // 전송 간격 대기

            // CAN 프레임 전송
            if (write(sockfd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                perror("CAN 프레임 전송 실패");
                close(sockfd);
                return 1;
            }

            // 현재 시간 확인
            current_time = time(NULL);
            int elapsed_seconds = (int)difftime(current_time, start_time);

            // 1초마다 한 번씩만 출력
            if (elapsed_seconds > last_printed_time) {
                printf("패킷 전송 시작 후 경과 시간: %d초\n", elapsed_seconds);
                last_printed_time = elapsed_seconds;
            }
        }
    }

    // 소켓 종료
    close(sockfd);
    return 0;
}
