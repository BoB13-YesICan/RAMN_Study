#ifndef ATTACK_PACKET_SENDER_H
#define ATTACK_PACKET_SENDER_H

/*================================================================
attack_packet_sender.h
==================================================================*/

#include "all_headers.h"
#include "payloads.h"

//attack_packet_sender
void attack_packet_sender(int socket, struct sockaddr_can *addr, int attack_code, int canid, int time_diff);

#endif // ATTACK_PACKET_SENDER_H
