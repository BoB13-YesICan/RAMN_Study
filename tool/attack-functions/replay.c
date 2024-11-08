#include "all_headers.h"

/*================================================================
replay.c
This function replays CAN packets with or without payloads.

functions:
- replay_replay: replays CAN packets with input payloads
- replay_suddenaccel: replays CAN packets with payloads (accel 100% && pedal 100%)

Parameters:
- socket: CAN socket
- addr: CAN address
==================================================================*/
#define MAX_BYTES 8

void replay_replay(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    struct can_frame tx_frame;
    int data_len = 0;
    size_t input_length = 0;
    char hex_input[17];
    uint8_t byte_array[MAX_BYTES] = {0};

    while (1) {
        printf("Enter payload to send in hexadecimal (e.g., OA1B2C ...), up to 8 bytes\n");
        printf("type here >> ");
        scanf("%16s", hex_input);

        input_length = strlen(hex_input);
        if (input_length % 2 == 0 && input_length <= 16) {
            data_len = input_length / 2;
            break;
        } else printf("\nwrong payload data, please enter maximum 8-byte hex data.\n");
    }

    for (size_t i = 0; i < input_length / 2; i++) {
        sscanf(hex_input + 2 * i, "%2hhx", &tx_frame.data[i]);
    }
    while(1) {
        send_can_packet_showpayload(socket, addr, tx_frame.data, data_len, canid);
        if (time_diff == 0) break;
        usleep (time_diff * 1000);
    }  
}


void replay_suddenaccel(int socket, struct sockaddr_can *addr, int canid, int time_diff){
    do {
        send_can_packet(socket, addr, replay.payload1, replay.payload1_len, 0x24);
        send_can_packet(socket, addr, replay.payload2, replay.payload2_len, 0x39);
        if (time_diff == 0) break;
        usleep(time_diff * 1000);
    } while (1);
}
