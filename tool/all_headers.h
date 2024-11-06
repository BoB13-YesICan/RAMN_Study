#ifndef ALL_HEADERS_H
#define ALL_HEADERS_H

/*================================================================
all_headers.h
==================================================================*/

//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>
#include <errno.h>


//userdefined headers
#include "payloads.h"
#include "attack_packet_sender.h"

//==============================individual attack functions=================================
//dos.c
void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff);

//replay.c
void replay_replay(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void replay_suddenaccel(int socket, struct sockaddr_can *addr, int canid, int time_diff);

//fuzzing.c
void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void fuzzing_random_payload(int socket, struct sockaddr_can *addr, int canid, int time_diff);

//suspension.c
void suspension_resetecu(int socket, struct sockaddr_can *addr, int canid, int time_diff);

//masquerade.c
void msq_msq(int socket, struct sockaddr_can *addr, int canid, int time_diff);


#endif // ALL_HEADERS_H