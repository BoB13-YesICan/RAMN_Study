#ifndef PAYLOADS_H
#define PAYLOADS_H

#include "all_headers.h"

/*================================================================
payloads.h
'payloads' structure variables declaraitons
all CAN Packets for send to victim is here
rest in payloads.c
==================================================================*/

struct payloads {
    unsigned char payload1[8];
    unsigned char payload2[8];
    unsigned char payload3[8];
    unsigned char payload4[8];
    size_t payload1_len;
    size_t payload2_len;
    size_t payload3_len;
    size_t payload4_len;
};

extern struct payloads msq;             //masquerade, masqueradeCheck
extern struct payloads fuzzing;         //findUDS, findBUDS, randomCANID, randomPayload
extern struct payloads dos;             //dos
extern struct payloads replay;          //replay, suddenaccel
extern struct payloads suspension;      //resetecu, dos(?)

#endif // PAYLOADS_H