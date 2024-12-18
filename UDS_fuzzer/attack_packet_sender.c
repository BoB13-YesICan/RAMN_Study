#include "all_headers.h"

/*================================================================
attack_packet_sender.c
This function calls different attack functions based on the given attack code

Parameters:
- socket: CAN socket
- addr: CAN address
- attack_code: attack code
- canid: CAN ID
- time_diff: time difference between CAN packets in milliseconds
==================================================================*/

// printf("=====================attack  codes======================\n");
// printf("(1): dos_dos                (2): replay_replay\n");
// printf("(3): replay_suddenaccel     (4): fuzzing_find_uds\n");
// printf("(5): fuzzing_random_canid   (6): fuzzing_random_payload\n");
// printf("(7): suspension_resetecu    (8): msq_msq[masquerade]\n");
// printf("(9): fuzzing_uds_service    (10): fuzzing_uds_sid_multicanid\n");
// printf("========================================================\n\n");

void attack_packet_sender(int socket, struct sockaddr_can *addr, int attack_code, int canid, int time_diff) {

    printf(RED_TEXT"=====================attack  start======================\n");
    printf(RESET_COLOR"\n");

    //call attack-functions
    // dos, replay, fuzzing, suspension, msq
    switch (attack_code) {
        case 1:
            dos_dos(socket, addr, canid, time_diff);
            break;
        case 2:
            replay_replay(socket, addr, canid, time_diff);
            break;
        case 3:
            replay_suddenaccel(socket, addr, canid, time_diff);
            break;
        case 4:
            fuzzing_find_uds(socket, addr, canid, time_diff);
            break;
        case 5:
            fuzzing_random_canid(socket, addr, canid, time_diff);
            break;
        case 6:
            fuzzing_random_payload(socket, addr, canid, time_diff);
            break;
        case 7:
            suspension_resetecu(socket, addr, canid, time_diff);
            break;
        case 8:
            msq_msq(socket, addr, canid, time_diff);
            break;
        case 9:
            mode_f(socket, addr, canid, time_diff);
            break;
        case 10:
            mode_fs(socket, addr, canid, time_diff);
            break;

        default:
            printf("Invalid attack code\n");
            break;
    }
}

