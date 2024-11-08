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
#include <signal.h>
#include <sys/wait.h>


//userdefined headers
#include "payloads.h"
#include "attack_packet_sender.h"
#include "attack-functions/attack_functions.h"


#endif // ALL_HEADERS_H