#ifndef PAYLOADS_H
#define PAYLOADS_H

#include "all_headers.h"

//payloads structure declaration
struct payloads {
    unsigned char payload1[3];
    unsigned char payload2[3];
    unsigned char payload3[8];
    unsigned char payload4[8];
};

/*================================================================
'payloads' structure variables declaraitons
all CAN Packets for send to victim is here
rest in payloads.c
==================================================================*/

extern struct payloads msq;             //masquerade, masqueradeCheck
extern struct payloads fuzzing;         //findUDS, findBUDS, randomCANID, randomPayload
extern struct payloads dos;             //dos
extern struct payloads replay;          //replay, suddenaccel
extern struct payloads suspension;      //resetecu, dos(?)



// CAN pakcet send function
void send_can_packet(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id);

#endif // PAYLOADS_H
