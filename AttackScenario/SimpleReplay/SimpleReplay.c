#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#define DEFAULT_INTERVAL_MS 10
#define MAX_DATA_BYTES 8

//ms unit delay
void delay_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("usage: %s [interface name]] [ECUID#payload] -diff [ms]\n", argv[0]);
        printf("example: %s can0 7E0#1102030405060708 -diff 10\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    time_t start_time, current_time;
    int last_printed_time = 0;
    int interval_ms = -1;  //-diff option recognize

    //args handling
    char *interface_name = argv[1];
    char *id_and_data = argv[2];
    char *data_str;

    char *delimiter = strchr(id_and_data, '#');
    if (delimiter == NULL) {
        printf("wrong payload format.\n");
        return 1;
    }

    *delimiter = '\0'; // '#'into null, serperate payload
    unsigned int ecu_id = (unsigned int)strtol(id_and_data, NULL, 16);  //CANID into hex
    data_str = delimiter + 1;  //payload

    int data_length = strlen(data_str);
    if (data_length <= 0 || data_length > MAX_DATA_BYTES * 2 || data_length % 2 != 0) {
        printf("wrong data format.\n");
        return 1;
    }

    frame.can_dlc = data_length / 2;
    for (int i = 0; i < frame.can_dlc; i++) {
        sscanf(&data_str[i * 2], "%2hhx", &frame.data[i])
    }

    if (argc > 4 && strcmp(argv[3], "-diff") == 0) {
        interval_ms = atoi(argv[4]);
    }


    if ((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket generate fail");
        return 1;
    }

    strcpy(ifr.ifr_name, interface_name);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl fail");
        close(sockfd);
        return 1;
    }


    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;


    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("socket bind fail");
        close(sockfd);
        return 1;
    }


    frame.can_id = ecu_id;


    start_time = time(NULL);


    if (write(sockfd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("CAN frame send fail");
        close(sockfd);
        return 1;
    }

    printf("send packet\n");


    if (interval_ms > 0) {
        while (1) {
            delay_ms(interval_ms);


            if (write(sockfd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
                perror("CAN frame send fail");
                close(sockfd);
                return 1;
            }

            current_time = time(NULL);
            int elapsed_seconds = (int)difftime(current_time, start_time);


            if (elapsed_seconds > last_printed_time) {
                printf("time pass until sending packets: %dsec\n", elapsed_seconds);
                last_printed_time = elapsed_seconds;
            }
        }
    }


    close(sockfd);
    return 0;
}
