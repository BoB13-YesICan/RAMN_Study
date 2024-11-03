#include "all_headers.h"

/*================================================================
main
==================================================================*/

int main() {
    char interface[IFNAMSIZ];
    int attack_code;
    int canid;
    int time_diff;

    //get user input
    printf("Enter CAN interface name (e.g., vcan0): ");
    scanf("%s", interface);
    
    printf("Enter attack code (1 or 2): ");
    scanf("%d", &attack_code);

    printf("Enter CAN ID (in hexadecimal, e.g., 0x100): ");
    scanf("%x", &canid);

    printf("Enter time difference (in ms): ");
    scanf("%d", &time_diff);

    //call attack_packet_sender
    attack_packet_sender(interface, attack_code, canid, time_diff);

    return 0;
}
