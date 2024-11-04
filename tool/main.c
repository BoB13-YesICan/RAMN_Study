#include "all_headers.h"

/*================================================================
main
==================================================================*/

void print_get_attack_codes(void){
    printf("============attack codes=========\n ");
    printf("(1): dos_dos\n ");
    printf("(2): replay_replay\n");
    printf("(3): replay_suddenaccel\n");
    printf("(4): fuzzing_find_uds\n");
    printf("(5): fuzzing_random_canid\n");
    printf("(6): fuzzing_random_payload\n");
    printf("(7): suspension_resetecu\n");
    printf("(8): msq_msq[masquerade]\n");
    printf("=================================\n");
}

int main() {
    char interface[IFNAMSIZ];
    int attack_code;
    int canid;
    int time_diff;

    // 사용자 입력 받기
    printf("Enter CAN interface name (e.g., vcan0): ");
    scanf("%s", interface);
    
    print_get_attack_codes();
    scanf("%d", &attack_code);

    printf("Enter CAN ID (in hexadecimal, [e.x:0x100]): ");
    scanf("%x", &canid);

    printf("Enter time difference (in ms): ");
    scanf("%d", &time_diff);

    // 소켓 생성 및 설정
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Socket creation error");
        return 1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX error");
        close(s);
        return 1;
    }

    struct sockaddr_can addr = {0};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Socket binding error");
        close(s);
        return 1;
    }

    // 소켓과 주소를 attack_packet_sender로 전달
    attack_packet_sender(s, &addr, attack_code, canid, time_diff);

    close(s);  // 메인 함수에서 소켓 닫기
    return 0;
}
