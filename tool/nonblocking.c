#include "all_headers.h"

/*================================================================
nonblocking.c
'payloads' structure variables declaraitons
all CAN Packets for send to victim is here
rest in payloads.c
==================================================================*/

void set_nonblocking(int fd) {
    int flags;
    //get file descriptor flag
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
        perror("fcntl F_GETFL failed");
        exit(1);
    }
    //set nonblocking mode
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        exit(1);
    }
}

void set_blocking(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
        perror("fcntl F_GETFL failed");
        exit(EXIT_FAILURE);
    }

    //erase nonblocking flag to set blocking moded.
    flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL failed");
        exit(EXIT_FAILURE);
    }
}