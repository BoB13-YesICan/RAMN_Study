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
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [can_id] -diff [time_interval]\n", argv[0]);
        return 1;
    }

    char *interface = argv[1];
    int can_id = (int)strtol(argv[2], NULL, 16);
    int interval = atoi(argv[4]);

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

    // CAN ID 설정
    frame.can_id = can_id;
    frame.can_dlc = 3;

    // 페이로드 설정
    unsigned char payload1[3] = {0x02, 0x10, 0x02};
    unsigned char payload2[3] = {0x02, 0x11, 0x01};

    do {
        // 첫 번째 페이로드 전송 (021002)
        memcpy(frame.data, payload1, 3);
        send_can_frame(s, frame, addr);
        printf("Sent CAN frame with ID 0x%X and payload: %02X %02X %02X\n",
               frame.can_id, frame.data[0], frame.data[1], frame.data[2]);

        // 두 번째 페이로드 전송 (021101)
        memcpy(frame.data, payload2, 3);
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
