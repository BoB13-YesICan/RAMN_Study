#ifndef ATTACK_SENDER_H
#define ATTACK_SENDER_H

void attack_packet_sender(int socket, struct stockaddr &addr, int attack_code, int canid, int time_diff);

#endif // ATTACK_SENDER_H
