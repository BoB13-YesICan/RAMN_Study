#ifndef ATTACK_PACKET_SENDER_H
#define ATTACK_PACKET_SENDER_H

#include "all_headers.h"
#include "payloads.h"

//attack_packet_sender
void attack_packet_sender(int socket, struct sockaddr_can *addr, int attack_code, int canid, int time_diff);

//attack functions
void dos_dos(int socket, struct sockaddr_can *addr, int canid);
void replay_replay(int socket, struct sockaddr_can *addr, int canid);
void replay_suddenaccel(int socket, struct sockaddr_can *addr, int canid);
void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid);
void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid);
void fuzzing_random_payload(int socket, struct sockaddr_can *addr, int canid);
void suspension_resetecu(int socket, struct sockaddr_can *addr, int canid);
void msq_msq(int socket, struct sockaddr_can *addr, int canid, int time_diff);

#endif // ATTACK_PACKET_SENDER_H
