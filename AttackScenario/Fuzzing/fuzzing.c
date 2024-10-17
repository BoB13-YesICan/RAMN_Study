#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define START_CAN_ID 0x100
#define END_CAN_ID   0x200
#define RESPONSE_OFFSET 0x008

// HOWTOUSE: ' fuzzing [can interface name] '

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <can_interface>\n", argv[0]);
        return 1;
    }

    char *can_if = argv[1];
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    struct can_frame response;
    fd_set read_fds;
    struct timeval timeout;

    // 소켓 생성
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("socket");
        return 1;
    }

    strcpy(ifr.ifr_name, can_if);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // CAN ID 순회하며 페이로드 전송
    for (int can_id = START_CAN_ID; can_id <= END_CAN_ID; ++can_id) {
        // 전송할 프레임 설정
        frame.can_id = can_id;
        frame.can_dlc = 3;
        frame.data[0] = 0x02;
        frame.data[1] = 0x3e;
        frame.data[2] = 0x00;

        // 프레임 전송
        if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("write");
            return 1;
        }

        // 응답 대기
        FD_ZERO(&read_fds);
        FD_SET(s, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(s + 1, &read_fds, NULL, NULL, &timeout);

        if (ret < 0) {
            perror("select");
            return 1;
        } else if (ret == 0) {
            // 타임아웃 - 응답 없음
            continue;
        }

        // 응답 수신
        if (read(s, &response, sizeof(struct can_frame)) < 0) {
            perror("read");
            return 1;
        }

        // 기대하는 응답인지 확인
        if (response.can_id == (can_id + RESPONSE_OFFSET) &&
            response.can_dlc == 3 &&
            response.data[0] == 0x02 &&
            response.data[1] == 0x7e &&
            response.data[2] == 0x00) {
            printf("응답 받은 CAN ID: 0x%03X\n", response.can_id);
        }
    }

    close(s);
    return 0;
}
