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
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define RX_CANID_OFFSET 0x008

// payloads array
unsigned char checkecupayload[3] = {0x02, 0x3e, 0x00};
unsigned char resetpayload1[3] = {0x02, 0x10, 0x02};
unsigned char resetpayload2[3] = {0x02, 0x11, 0x01};
unsigned char accelpayload1[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};
unsigned char accelpayload2[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};

//shared memory structure
typedef struct {
    int socket_fd;
    struct sockaddr_can addr;
    uint32_t ECUcan_id;
    unsigned int interval;
    pthread_mutex_t write_mutex; //mutex only for write
    volatile int running; //variable to control thread execution state
} shared_data_t;

//global variable for signal handler
volatile sig_atomic_t stop = 0;

//func for signal handler
void handle_sigint(int sig) {
    stop = 1;
}

//CAN frame tx func(protects write operate only)
void send_can_frame(shared_data_t *data, struct can_frame *frame) {
    pthread_mutex_lock(&data->write_mutex);
    ssize_t nbytes = write(data->socket_fd, frame, sizeof(struct can_frame));
    if (nbytes != sizeof(struct can_frame)) {
        perror("Write fail");
    } else {
        printf("Sent CAN ID: 0x%X, DLC: %d, Data: ", frame->can_id, frame->can_dlc);
        for(int i = 0; i < frame->can_dlc; i++) {
            printf("%02X ", frame->data[i]);
        }
        printf("\n");
    }
    pthread_mutex_unlock(&data->write_mutex);
}

//sender thread func
void* sender_thread(void* arg) {
    shared_data_t *data = (shared_data_t*)arg;
    struct can_frame frame;

    printf("Sender thread started.\n");

    while (data->running) {
        //send accelpayload1
        frame.can_id = 0x24;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload1, 8);
        printf("Sending accelpayload1\n");
        send_can_frame(data, &frame);

        //send accelpayload2
        frame.can_id = 0x39;
        frame.can_dlc = 8;
        memcpy(frame.data, accelpayload2, 8);
        printf("Sending accelpayload2\n");
        send_can_frame(data, &frame);

        if (data->interval > 0) {
            usleep(data->interval * 10000); // interval * 10ms
        } else {
            usleep(10000); //default sleep interval(10ms)
        }
    }

    printf("Sender thread exiting.\n");
    pthread_exit(NULL);
}

//receiver thread func
void* receiver_thread(void* arg) {
    shared_data_t *data = (shared_data_t*)arg;
    struct can_frame rx_frame;
    struct can_frame reset_frame;
    ssize_t nbytes;

    printf("Receiver thread started.\n");

    while (data->running) {
        // Non-blocking read
        nbytes = read(data->socket_fd, &rx_frame, sizeof(struct can_frame));

        if (nbytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, wait and retry
                usleep(10000);  //retry after 10ms
                continue;
            } else {
                perror("Read");
                break;
            }
        }

        printf("Received CAN ID: 0x%X, DLC: %d, Data: ", rx_frame.can_id, rx_frame.can_dlc);
        for(int i = 0; i < rx_frame.can_dlc; i++) {
            printf("%02X ", rx_frame.data[i]);
        }
        printf("\n");

        // Check if ECU sent live packet
        if (rx_frame.can_id == (data->ECUcan_id + RX_CANID_OFFSET) &&
            rx_frame.can_dlc == 3 &&
            rx_frame.data[0] == 0x02 &&
            rx_frame.data[1] == 0x7e &&
            rx_frame.data[2] == 0x00) {
            
            printf("ECU alive, trying reset...\n");

            //send resetpayload1
            reset_frame.can_id = data->ECUcan_id;
            reset_frame.can_dlc = 3;
            memcpy(reset_frame.data, resetpayload1, 3);
            printf("Sending resetpayload1\n");
            send_can_frame(data, &reset_frame);

            //send resetpayload2
            reset_frame.can_id = data->ECUcan_id;
            reset_frame.can_dlc = 3;
            memcpy(reset_frame.data, resetpayload2, 3);
            printf("Sending resetpayload2\n");
            send_can_frame(data, &reset_frame);
        }
    }

    printf("Receiver thread exiting.\n");
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

    //set signal handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    //reset shared memory
    shared_data.socket_fd = -1;
    shared_data.ECUcan_id = ECUcan_id;
    shared_data.interval = interval;
    shared_data.running = 1; //reset thread operate state

    //reset mutex (write only)
    if (pthread_mutex_init(&shared_data.write_mutex, NULL) != 0) {
        perror("write_mutex init fail");
        exit(EXIT_FAILURE);
    }

    //generate CAN socket
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("generate socket fail");
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }
    shared_data.socket_fd = s;

    //set socket non-block mode
    int flags = fcntl(s, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL fail");
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }
    if (fcntl(s, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL fail");
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }

    //set network interface
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0){
        perror("IOCTL fail");
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("binding fail");
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }
    shared_data.addr = addr;

    // ECU check for entering diagnostic mode before starting threads
    printf("check if ecu can enter diag mode...\n");

    int max_attempts = 500;
    int attempt;
    int ecu_alive = 0;

    struct can_frame check_frame;
    struct can_frame rxcheck_frame;

    //set check frame
    check_frame.can_id = shared_data.ECUcan_id;
    check_frame.can_dlc = 3;
    memcpy(check_frame.data, checkecupayload, 3);

    for (attempt = 1; attempt <= max_attempts && !ecu_alive; attempt++) {
        printf("try %d: check frame send\n", attempt);
        send_can_frame(&shared_data, &check_frame);

        //wait maximum 2 sec
        time_t start_time = time(NULL);
        while (time(NULL) - start_time < 2) { //2sec timeout
            ssize_t nbytes = read(shared_data.socket_fd, &rxcheck_frame, sizeof(struct can_frame));
            if (nbytes > 0) {
                //read received frame
                if (rxcheck_frame.can_dlc == 3 &&
                    rxcheck_frame.data[0] == 0x02 &&
                    rxcheck_frame.data[1] == 0x7e &&
                    rxcheck_frame.data[2] == 0x00) {
                    printf("expected answer from ecu.\n");
                    ecu_alive = 1;
                    break;
                }
            } else if (nbytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read fail");
                break;
            }
            usleep(100000); //wait for 100ms
        }

        if (ecu_alive) {
            printf("ECU ready, activate thread\n");
            break;
        } else {
            printf("no expected response, retry...\n");
        }
    }

    if (!ecu_alive) {
        fprintf(stderr, "ECU no reponse, end program\n");
        close(shared_data.socket_fd);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }

    //execute thread
    ret = pthread_create(&sender_tid, NULL, sender_thread, &shared_data);
    if (ret != 0) {
        fprintf(stderr, "tx thread init fail: %s\n", strerror(ret));
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&receiver_tid, NULL, receiver_thread, &shared_data);
    if (ret != 0) {
        fprintf(stderr, "rx thread init fail: %s\n", strerror(ret));
        shared_data.running = 0;
        pthread_cancel(sender_tid);
        pthread_join(sender_tid, NULL);
        close(s);
        pthread_mutex_destroy(&shared_data.write_mutex);
        exit(EXIT_FAILURE);
    }

    printf("Program started. Press Ctrl+C to exit.\n");

    //main thread wait for signal
    while (!stop) {
        sleep(1);
    }

    printf("Termination signal received. Shutting down...\n");

    //kill thread after ctrlC
    shared_data.running = 0;

    //wait for tx thread ends
    pthread_join(sender_tid, NULL);

    //wait for rx thread ends
    pthread_join(receiver_tid, NULL);

    //clear whole resource
    pthread_mutex_destroy(&shared_data.write_mutex);
    close(s);
    printf("Program terminated gracefully.\n");
    return 0;
}
