#ifndef ATTACK_FUNCTIONS_H
#define ATTACK_FUNCTIONS_H

#include "all_headers.h"

void dos_dos(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void replay_replay(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void replay_suddenaccel(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void fuzzing_find_uds(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void fuzzing_random_canid(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void fuzzing_random_payload(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void suspension_resetecu(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void msq_msq(int socket, struct sockaddr_can *addr, int canid, int time_diff);
void mode_f(int socket, struct sockaddr_can *addr, int canid, int time_diff);

#endif // ATTACK_FUNCTIONS_H
