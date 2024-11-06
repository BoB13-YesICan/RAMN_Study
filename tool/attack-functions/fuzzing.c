#include "all_headers.h"

#define END_CANID 0x555

void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff){

}

void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    for (int tx_can_id = canid; tx_can_id <= END_CANID; tx_can_id++) {
        send_can_packet(socket, addr, fuzzing.payload3, fuzzing.payload3_len, canid);
        canid ++;

        if (time_diff == 0) break;
        usleep(time_diff*1000);
    }
}

void fuzzing_random_payload(int socket, struct sockaddr_can *addr, int canid, int time_diff){

}