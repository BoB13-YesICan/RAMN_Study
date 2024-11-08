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

/*================================================================
(1): dos_dos                (5): fuzzing_random_canid
(2): replay_replay          (6): fuzzing_random_payload
(3): replay_suddenaccel     (7): suspension_resetecu
(4): fuzzing_find_uds       (8): msq_msq[masquerade]
==================================================================*/

void attack_packet_sender(int socket, struct sockaddr_can *addr, int attack_code, int canid, int time_diff) {

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
            return 1;
            

        default:
            printf("Invalid attack code\n");
            break;
    }
}

