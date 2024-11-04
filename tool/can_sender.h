#ifndef CAN_SENDER_H
#define CAN_SENDER_H

void send_can_packet_showpayload(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id);
void send_can_packet_sendonly(int socket, struct sockaddr_can *addr, unsigned char *data, int length, int can_id);

#endif // cAN_SENDER_H74