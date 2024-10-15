#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

void send_can_frame(int s, struct can_frame frame, struct sockaddr_can addr) {
    if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("Write");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[2], "-diff") != 0) {
        printf("Usage: %s [can_interface] -diff [time_interval]\n", argv[0]);
        return 1;
    }

    char *interface = argv[1];
    int interval = atoi(argv[2]);

    // CAN 소켓 생성
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket");
        return 1;
    }
    
    strcpy(ifr.ifr_name, interface);
    ioctl(s, SIOCGIFINDEX, &ifr);
    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    // CAN ID와 페이로드 설정
    unsigned char payload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
    unsigned char payload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};

    do {
        // CAN ID 0x24 페이로드 전송 (0000DF9D35EE1212)
        frame.can_id = 0x24;
        frame.can_dlc = 8;
        memcpy(frame.data, payload1, 8);
        send_can_frame(s, frame, addr);

        // CAN ID 0x39 페이로드 전송 (0FC5D399DD211212)
        frame.can_id = 0x39;
        frame.can_dlc = 8;
        memcpy(frame.data, payload2, 8);
        send_can_frame(s, frame, addr);

        // 사이클 간격 대기
        if (interval == 0) {
            break;
        }

        usleep(interval * 1000); // ms 단위로 간격 유지

    } while (1);

    close(s);
    return 0;
}