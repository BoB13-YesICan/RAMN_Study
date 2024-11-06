#ifndef CAN_SENDER_H
#define CAN_SENDER_H

/*================================================================
can_sender.h
==================================================================*/
#include "all_headers.h"

void send_can_packet_showpayload(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id);
void send_can_packet(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id);

#endif // CAN_SENDER_H74