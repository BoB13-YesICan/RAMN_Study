#include "all_headers.h"

/*================================================================
main.c
This program is a CAN packet sender with various attack codes.

Parameters:
- interface: CAN interface name
- attack_code: attack code
- canid: CAN ID (in hexadecimal)
- time_diff: time difference between packets (in ms)
==================================================================*/

void print_get_attack_codes(void){
    printf("=====================attack  codes=====================\n ");
    printf("(1): dos_dos                (5): fuzzing_random_canid\n ");
    printf("(2): replay_replay          (6): fuzzing_random_payload\n");
    printf("(3): replay_suddenaccel     (7): suspension_resetecu\n");
    printf("(4): fuzzing_find_uds       (8): msq_msq[masquerade]\n");
    printf("========================================================\n");
}

int main() {
    char interface[IFNAMSIZ];
    int attack_code;
    int canid;
    int time_diff;

    // 사용자 입력 받기 
    print_get_attack_codes();
    scanf("%d", &attack_code);

    printf("Enter CAN interface name (e.g., vcan0): ");
    scanf("%s", interface);

    printf("Enter CAN ID (in hexadecimal, [e.x:0x100]): ");
    scanf("%x", &canid);

    printf("Enter time difference (in ms), [enter 0 to send once]: ");
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

    //call attack_packet_sender
    attack_packet_sender(s, &addr, attack_code, canid, time_diff);
    
    //close the socket
    close(s);
    return 0;
}
