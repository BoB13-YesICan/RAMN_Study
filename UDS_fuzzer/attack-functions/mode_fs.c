/*================================================================
mode_fs.c
This function fuzzes UDS services for multiple CANIDs, sending all defined payloads for each service,
processing responses based on uds_reply, and saving output to separate CSV files
in the csv_multiCANID_report[timestamp] directory, with each CANID having its own subdirectory.
=================================================================*/

#include "all_headers.h"
#include "payloads.h" // To access uds_reply and other payload structures
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define SLEEP_TIME 100000 // 100,000 microseconds = 100 milliseconds

/*================================================================
Function Prototypes
=================================================================*/
void send_payload_and_process_response(int socket, struct sockaddr_can *addr, int canid, 
                                      const char *servicename, const char *optionname, 
                                      unsigned char *payload, size_t payload_len, FILE *csv_file);

int is_placeholder_payload(unsigned char *payload, size_t len);

/*================================================================
mode_fs
=================================================================*/
void mode_fs(int socket, struct sockaddr_can *addr, int start_canid, int time_diff) {
    // Generate timestamp for the directory name
    struct timeval tv_dir;
    gettimeofday(&tv_dir, NULL);
    struct tm *tm_info_dir = localtime(&tv_dir.tv_sec);
    int tenths_dir = tv_dir.tv_usec / 100000; // 0.1 second unit

    // Timestamp: YYYYMMDD_HHMMSS.T
    char timestamp_dir[32];
    snprintf(timestamp_dir, sizeof(timestamp_dir), "%04d%02d%02d_%02d%02d%02d.%d",
             tm_info_dir->tm_year + 1900, tm_info_dir->tm_mon + 1, tm_info_dir->tm_mday,
             tm_info_dir->tm_hour, tm_info_dir->tm_min, tm_info_dir->tm_sec, tenths_dir);

    // Directory name: csv_multiCANID_report[timestamp]
    char report_dir[256];
    snprintf(report_dir, sizeof(report_dir), "csv_multiCANID_report[%s]", timestamp_dir);

    // Create the report directory
    struct stat st_report = {0};
    if (stat(report_dir, &st_report) == -1) {
        if (mkdir(report_dir, 0700) != 0) {
            perror("Failed to create csv_multiCANID_report directory");
            return;
        }
    }

    printf("CSV reports will be saved in directory: %s\n", report_dir);

    // Array of pointers to all defined UDS_request structures
    struct uds_request *all_services[] = {
        &diagnostic_session_control,    // Service 0x10
        &ECU_reset,                     // Service 0x11
        &clear_diagnostic_information,  // Service 0x14
        &read_dtc_information,          // Service 0x19
        &read_data_by_identifier,       // Service 0x22
        &read_memory_by_address,        // Service 0x23
        &security_access,               // Service 0x27
        &write_data_by_identifier,      // Service 0x2E
        &routine_control,               // Service 0x31
        &request_download,              // Service 0x34
        &request_upload,                // Service 0x35
        &transfer_data,                 // Service 0x36
        &request_transfer_exit,         // Service 0x37
        &write_memory_by_address_3D,    // Service 0x3D
        &tester_present,                // Service 0x3E
        &control_dtc_settings,          // Service 0x85
        &link_control                   // Service 0x87
    };

    int num_all_services = sizeof(all_services) / sizeof(all_services[0]);

    // Iterate over CANIDs from start_canid to 0x7FF
    for(int current_canid = start_canid; current_canid <= 0x7FF; current_canid++) {
        printf("\nProcessing CANID=0x%03X\n", current_canid);

        // **Create Subdirectory for Each CANID**
        char canid_dir[64];
        snprintf(canid_dir, sizeof(canid_dir), "%s/0x%03X", report_dir, current_canid);

        // Create CANID subdirectory
        struct stat st_canid = {0};
        if (stat(canid_dir, &st_canid) == -1) {
            if (mkdir(canid_dir, 0700) != 0) {
                perror("Failed to create CANID subdirectory");
                continue;
            }
        }

        // **Create CSV filenames within CANID subdirectory**
        char csv_filename_normal[256], csv_filename_psession[256], csv_filename_esession[256];
        snprintf(csv_filename_normal, sizeof(csv_filename_normal), "%s/0x%03X_normal.csv", canid_dir, current_canid);
        snprintf(csv_filename_psession, sizeof(csv_filename_psession), "%s/0x%03X_Psession.csv", canid_dir, current_canid);
        snprintf(csv_filename_esession, sizeof(csv_filename_esession), "%s/0x%03X_Esession.csv", canid_dir, current_canid);

        // **Open CSV files for writing**
        FILE *normal_csv = fopen(csv_filename_normal, "w");
        if (normal_csv == NULL) {
            perror("Failed to open Normal CSV file");
            continue;
        }
        fprintf(normal_csv, "Timestamp,CanID,Servicename,Optionname,Answer,ErrorCode,ResponseMessage\n");

        FILE *Psession_csv = fopen(csv_filename_psession, "w");
        if (Psession_csv == NULL) {
            perror("Failed to open Psession CSV file");
            fclose(normal_csv);
            continue;
        }
        fprintf(Psession_csv, "Timestamp,CanID,Servicename,Optionname,Answer,ErrorCode,ResponseMessage\n");

        FILE *Esession_csv = fopen(csv_filename_esession, "w");
        if (Esession_csv == NULL) {
            perror("Failed to open Esession CSV file");
            fclose(normal_csv);
            fclose(Psession_csv);
            continue;
        }
        fprintf(Esession_csv, "Timestamp,CanID,Servicename,Optionname,Answer,ErrorCode,ResponseMessage\n");

        // **Set socket receive timeout to 100ms**
        struct timeval timeout = {0, SLEEP_TIME}; // 0 seconds, 100,000 microseconds
        if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("Set socket options error");
            fclose(normal_csv);
            fclose(Psession_csv);
            fclose(Esession_csv);
            continue;
        }

        /*================================================================
        Normal Session: Send all services including the focused payload
        =================================================================*/
        for(int s = 0; s < num_all_services; s++) {
            struct uds_request *service = all_services[s];
            const char *servicename = NULL;

            // Determine the service name based on the pointer
            if (service == &diagnostic_session_control) servicename = "Diagnostic Session Control";
            else if (service == &ECU_reset) servicename = "ECU Reset";
            else if (service == &clear_diagnostic_information) servicename = "Clear Diagnostic Information";
            else if (service == &read_dtc_information) servicename = "Read DTC Information";
            else if (service == &read_data_by_identifier) servicename = "Read Data By Identifier";
            else if (service == &read_memory_by_address) servicename = "Read Memory By Address";
            else if (service == &security_access) servicename = "Security Access";
            else if (service == &write_data_by_identifier) servicename = "Write Data By Identifier";
            else if (service == &routine_control) servicename = "Routine Control";
            else if (service == &request_download) servicename = "Request Download";
            else if (service == &request_upload) servicename = "Request Upload";
            else if (service == &transfer_data) servicename = "Transfer Data";
            else if (service == &request_transfer_exit) servicename = "Request Transfer Exit";
            else if (service == &write_memory_by_address_3D) servicename = "Write Memory By Address (0x3D)";
            else if (service == &tester_present) servicename = "Tester Present";
            else if (service == &control_dtc_settings) servicename = "Control DTC Settings";
            else if (service == &link_control) servicename = "Link Control";
            else servicename = "Unknown Service";

            // Iterate through each payload in the service
            for(int p = 0; p < 5; p++) { // payload1 to payload5
                unsigned char *current_payload = NULL;
                size_t payload_len = 0;
                const char *optionname = NULL;

                switch(p) {
                    case 0:
                        current_payload = service->payload1;
                        optionname = service->service1;
                        payload_len = (service->payload1_len > 0) ? service->payload1_len : 0;
                        break;
                    case 1:
                        current_payload = service->payload2;
                        optionname = service->service2;
                        payload_len = (service->payload2_len > 0) ? service->payload2_len : 0;
                        break;
                    case 2:
                        current_payload = service->payload3;
                        optionname = service->service3;
                        payload_len = (service->payload3_len > 0) ? service->payload3_len : 0;
                        break;
                    case 3:
                        current_payload = service->payload4;
                        optionname = service->service4;
                        payload_len = (service->payload4_len > 0) ? service->payload4_len : 0;
                        break;
                    case 4:
                        current_payload = service->payload5;
                        optionname = service->service5;
                        payload_len = (service->payload5_len > 0) ? service->payload5_len : 0;
                        break;
                    default:
                        continue;
                }

                // Skip if payload length is 0 (placeholder)
                if(payload_len == 0 || is_placeholder_payload(current_payload, payload_len)) {
                    continue;
                }

                // Optional: Sleep between payloads
                if (time_diff > 0) {
                    usleep(time_diff * 1000); // Convert ms to µs
                }

                // Send the current payload and process the response
                send_payload_and_process_response(socket, addr, current_canid, servicename, optionname, current_payload, payload_len, normal_csv);
            }
        }

        /*================================================================
        Psession (Programming Session):
        1. Send the Psession initialization payload (0x02, 0x10, 0x02)
        2. Send all payloads excluding the focused payload
        =================================================================*/
        // 1. Send Psession initialization payload
        unsigned char Psession_payload[] = {0x02, 0x10, 0x02};
        send_payload_and_process_response(socket, addr, current_canid, "Diagnostic Session Control", "Programming Session", Psession_payload, sizeof(Psession_payload), Psession_csv);

        // 2. Send all services except Diagnostic Session Control
        for(int s = 0; s < num_all_services; s++) {
            struct uds_request *service = all_services[s];
            // Skip Diagnostic Session Control
            if (service == &diagnostic_session_control) continue;

            const char *servicename = NULL;

            // Determine the service name based on the pointer
            if (service == &ECU_reset) servicename = "ECU Reset";
            else if (service == &clear_diagnostic_information) servicename = "Clear Diagnostic Information";
            else if (service == &read_dtc_information) servicename = "Read DTC Information";
            else if (service == &read_data_by_identifier) servicename = "Read Data By Identifier";
            else if (service == &read_memory_by_address) servicename = "Read Memory By Address";
            else if (service == &security_access) servicename = "Security Access";
            else if (service == &write_data_by_identifier) servicename = "Write Data By Identifier";
            else if (service == &routine_control) servicename = "Routine Control";
            else if (service == &request_download) servicename = "Request Download";
            else if (service == &request_upload) servicename = "Request Upload";
            else if (service == &transfer_data) servicename = "Transfer Data";
            else if (service == &request_transfer_exit) servicename = "Request Transfer Exit";
            else if (service == &write_memory_by_address_3D) servicename = "Write Memory By Address (0x3D)";
            else if (service == &tester_present) servicename = "Tester Present";
            else if (service == &control_dtc_settings) servicename = "Control DTC Settings";
            else if (service == &link_control) servicename = "Link Control";
            else servicename = "Unknown Service";

            // Iterate through each payload in the service
            for(int p = 0; p < 5; p++) { // payload1 to payload5
                unsigned char *current_payload = NULL;
                size_t payload_len = 0;
                const char *optionname = NULL;

                switch(p) {
                    case 0:
                        current_payload = service->payload1;
                        optionname = service->service1;
                        payload_len = (service->payload1_len > 0) ? service->payload1_len : 0;
                        break;
                    case 1:
                        current_payload = service->payload2;
                        optionname = service->service2;
                        payload_len = (service->payload2_len > 0) ? service->payload2_len : 0;
                        break;
                    case 2:
                        current_payload = service->payload3;
                        optionname = service->service3;
                        payload_len = (service->payload3_len > 0) ? service->payload3_len : 0;
                        break;
                    case 3:
                        current_payload = service->payload4;
                        optionname = service->service4;
                        payload_len = (service->payload4_len > 0) ? service->payload4_len : 0;
                        break;
                    case 4:
                        current_payload = service->payload5;
                        optionname = service->service5;
                        payload_len = (service->payload5_len > 0) ? service->payload5_len : 0;
                        break;
                    default:
                        continue;
                }

                // Skip if payload length is 0 (placeholder)
                if(payload_len == 0 || is_placeholder_payload(current_payload, payload_len)) {
                    continue;
                }

                // Optional: Sleep between payloads
                if (time_diff > 0) {
                    usleep(time_diff * 1000); // Convert ms to µs
                }

                // Send the current payload and process the response
                send_payload_and_process_response(socket, addr, current_canid, servicename, optionname, current_payload, payload_len, Psession_csv);
            }
        }

        /*================================================================
        Esession (Extended Session):
        1. Send the Esession initialization payload (0x02, 0x10, 0x03)
        2. Send all payloads excluding the focused payload
        =================================================================*/
        // 1. Send Esession initialization payload
        unsigned char Esession_payload[] = {0x02, 0x10, 0x03};
        send_payload_and_process_response(socket, addr, current_canid, "Diagnostic Session Control", "Extended Session", Esession_payload, sizeof(Esession_payload), Esession_csv);

        // 2. Send all services except Diagnostic Session Control
        for(int s = 0; s < num_all_services; s++) {
            struct uds_request *service = all_services[s];
            // Skip Diagnostic Session Control
            if (service == &diagnostic_session_control) continue;

            const char *servicename = NULL;

            // Determine the service name based on the pointer
            if (service == &ECU_reset) servicename = "ECU Reset";
            else if (service == &clear_diagnostic_information) servicename = "Clear Diagnostic Information";
            else if (service == &read_dtc_information) servicename = "Read DTC Information";
            else if (service == &read_data_by_identifier) servicename = "Read Data By Identifier";
            else if (service == &read_memory_by_address) servicename = "Read Memory By Address";
            else if (service == &security_access) servicename = "Security Access";
            else if (service == &write_data_by_identifier) servicename = "Write Data By Identifier";
            else if (service == &routine_control) servicename = "Routine Control";
            else if (service == &request_download) servicename = "Request Download";
            else if (service == &request_upload) servicename = "Request Upload";
            else if (service == &transfer_data) servicename = "Transfer Data";
            else if (service == &request_transfer_exit) servicename = "Request Transfer Exit";
            else if (service == &write_memory_by_address_3D) servicename = "Write Memory By Address (0x3D)";
            else if (service == &tester_present) servicename = "Tester Present";
            else if (service == &control_dtc_settings) servicename = "Control DTC Settings";
            else if (service == &link_control) servicename = "Link Control";
            else servicename = "Unknown Service";

            // Iterate through each payload in the service
            for(int p = 0; p < 5; p++) { // payload1 to payload5
                unsigned char *current_payload = NULL;
                size_t payload_len = 0;
                const char *optionname = NULL;

                switch(p) {
                    case 0:
                        current_payload = service->payload1;
                        optionname = service->service1;
                        payload_len = (service->payload1_len > 0) ? service->payload1_len : 0;
                        break;
                    case 1:
                        current_payload = service->payload2;
                        optionname = service->service2;
                        payload_len = (service->payload2_len > 0) ? service->payload2_len : 0;
                        break;
                    case 2:
                        current_payload = service->payload3;
                        optionname = service->service3;
                        payload_len = (service->payload3_len > 0) ? service->payload3_len : 0;
                        break;
                    case 3:
                        current_payload = service->payload4;
                        optionname = service->service4;
                        payload_len = (service->payload4_len > 0) ? service->payload4_len : 0;
                        break;
                    case 4:
                        current_payload = service->payload5;
                        optionname = service->service5;
                        payload_len = (service->payload5_len > 0) ? service->payload5_len : 0;
                        break;
                    default:
                        continue;
                }

                // Skip if payload length is 0 (placeholder)
                if(payload_len == 0 || is_placeholder_payload(current_payload, payload_len)) {
                    continue;
                }

                // Optional: Sleep between payloads
                if (time_diff > 0) {
                    usleep(time_diff * 1000); // Convert ms to µs
                }

                // Send the current payload and process the response
                send_payload_and_process_response(socket, addr, current_canid, servicename, optionname, current_payload, payload_len, Esession_csv);
            }
        }

        // **Close all CSV files after processing all payloads for the current CANID**
        fclose(normal_csv);
        fclose(Psession_csv);
        fclose(Esession_csv);
    }

/*================================================================
Helper Function: is_placeholder_payload
=================================================================*/
int is_placeholder_payload(unsigned char *payload, size_t len) {
    for(int i = 0; i < (int)len; i++) { 
        if(payload[i] != 0x00) {
            return 0; // Not a placeholder
        }
    }
    return 1; // All bytes are 0x00, considered as placeholder
}

/*================================================================
Helper Function: send_payload_and_process_response
=================================================================*/
void send_payload_and_process_response(int socket, struct sockaddr_can *addr, int canid, 
                                      const char *servicename, const char *optionname, 
                                      unsigned char *payload, size_t payload_len, FILE *csv_file) {
    struct can_frame rx_frame;
    struct timespec start, current;
    char timestamp[64];
    const char *response_message = "-";
    const char *answer_type = "-";
    unsigned char error_code = 0x00;
    
    // Flag to track if any response was received
    int response_received = 0;

    // Extract service ID from the sent payload (assuming payload[1] is service ID)
    if (payload_len < 2) {
        printf("Invalid payload length for service: %s, option: %s\n", servicename, optionname);
        return;
    }
    unsigned char service_id = payload[1];
    
    // Prepare CAN frame for transmission
    struct can_frame tx_frame;
    memset(&tx_frame, 0, sizeof(struct can_frame));
    tx_frame.can_id = canid;
    tx_frame.can_dlc = (payload_len > 8) ? 8 : payload_len;
    memcpy(tx_frame.data, payload, tx_frame.can_dlc);
    
    // Log the CAN frame being sent
    printf("Sending CAN frame to CANID=0x%03X with payload length=%zu\n", canid, payload_len);
    
    // Print TX payloads
    printf("Payload: ");
    for(int i = 0; i < tx_frame.can_dlc; i++) {
        printf("0x%02X ", tx_frame.data[i]);
    }
    printf("\n");
    
    // Send the CAN frame
    if (write(socket, &tx_frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("Write");
        return;
    }
    
    // Record start time
    clock_gettime(CLOCK_REALTIME, &start);
    
    // Calculate expected response CANID
    int expected_canid = canid + 0x008;
    
    // Continuously read until timeout
    while(1) {
        int nbytes = read(socket, &rx_frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Timeout reached, no more responses
                break;
            } else {
                perror("Read");
                return;
            }
        } else if (nbytes == sizeof(struct can_frame)) {
            // Check if the received CANID matches the expected CANID
            if(rx_frame.can_id == expected_canid) { 
                // Check payload[1] for response type
                if(rx_frame.can_dlc < 2) {
                    printf("Received payload too short from CANID=0x%03X\n", rx_frame.can_id);
                    continue;
                }
                if(rx_frame.data[1] == (service_id + 0x40)) { // Positive Response
                    answer_type = "Positive";
                    // ResponseMessage remains '-' as per requirement
                }
                else if(rx_frame.data[1] == 0x7F) { // Negative Response
                    answer_type = "Reject";
                    if(rx_frame.can_dlc >= 4) { // Ensure payload[3] exists
                        error_code = rx_frame.data[3];
                        // Fetch response message from uds_reply based on error_code
                        switch(error_code) {
                            case 0x10:
                                response_message = uds_reply.answer10;
                                break;
                            case 0x11:
                                response_message = uds_reply.answer11;
                                break;
                            case 0x12:
                                response_message = uds_reply.answer12;
                                break;
                            case 0x13:
                                response_message = uds_reply.answer13;
                                break;
                            case 0x21:
                                response_message = uds_reply.answer21;
                                break;
                            case 0x22:
                                response_message = uds_reply.answer22;
                                break;
                            case 0x24:
                                response_message = uds_reply.answer24;
                                break;
                            case 0x31:
                                response_message = uds_reply.answer31;
                                break;
                            case 0x33:
                                response_message = uds_reply.answer33;
                                break;
                            case 0x50:
                                response_message = uds_reply.answer50;
                                break;
                            case 0x51:
                                response_message = uds_reply.answer51;
                                break;
                            case 0x67:
                                response_message = uds_reply.answer67;
                                break;
                            case 0xC5:
                                response_message = uds_reply.answerC5;
                                break;
                            case 0x7E:
                                response_message = uds_reply.answer7E;
                                break;
                            default:
                                response_message = "Unknown Negative Response";
                                break;
                        }
                    }
                    else {
                        response_message = "Invalid Negative Response Format";
                    }
                }
                else {
                    // Unknown response type, ignore
                    continue;
                }
                
                // Mark that a response was received
                response_received = 1;
                
                printf("Received %s from CANID=0x%03X, Error Code=0x%02X, Message=%s\n",
                       answer_type, rx_frame.can_id, error_code, response_message);
                
                // Generate timestamp: YYYY-MM-DD HH:MM:SS.T
                struct tm *tm_info_resp;
                time_t now_sec = time(NULL);
                tm_info_resp = localtime(&now_sec);
                struct timeval now_tv;
                gettimeofday(&now_tv, NULL);
                int tenths_now = now_tv.tv_usec / 100000; // 0.1 second unit
                snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%d",
                        tm_info_resp->tm_year + 1900, tm_info_resp->tm_mon + 1, tm_info_resp->tm_mday,
                        tm_info_resp->tm_hour, tm_info_resp->tm_min, tm_info_resp->tm_sec, tenths_now);
                
                // Write to CSV only if response is Positive or Reject
                if(strcmp(answer_type, "-") != 0) {
                    if(strcmp(answer_type, "Reject") == 0) {
                        fprintf(csv_file, "\"%s\",0x%03X,\"%s\",\"%s\",\"%s\",0x%02X,\"%s\"\n", 
                                timestamp, rx_frame.can_id, servicename, optionname, 
                                answer_type, error_code, response_message);
                    }
                    else { // Positive Response
                        fprintf(csv_file, "\"%s\",0x%03X,\"%s\",\"%s\",\"%s\",-,\"-\"\n", 
                                timestamp, rx_frame.can_id, servicename, optionname, 
                                answer_type);
                    }
                    fflush(csv_file); // Ensure data is written immediately
                }
                
            }
            // Else: Ignore other CANIDs
        }
        
        // Check elapsed time
        clock_gettime(CLOCK_REALTIME, &current);
        long elapsed_us = (current.tv_sec - start.tv_sec) * 1000000 + 
                          (current.tv_nsec - start.tv_nsec) / 1000;
        if(elapsed_us >= SLEEP_TIME) {
            // Timeout reached
            // No action needed here as loop will naturally exit
        }
        
        // Check if no response was received
        if(!response_received) {
            // Generate timestamp: YYYY-MM-DD HH:MM:SS.T
            struct tm *tm_info_no_resp;
            time_t now_sec_no_resp = time(NULL);
            tm_info_no_resp = localtime(&now_sec_no_resp);
            struct timeval now_tv_no_resp;
            gettimeofday(&now_tv_no_resp, NULL);
            int tenths_now_no_resp = now_tv_no_resp.tv_usec / 100000; // 0.1 second unit
            snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%d",
                    tm_info_no_resp->tm_year + 1900, tm_info_no_resp->tm_mon + 1, tm_info_no_resp->tm_mday,
                    tm_info_no_resp->tm_hour, tm_info_no_resp->tm_min, tm_info_no_resp->tm_sec, tenths_now_no_resp);
            
            printf("No Response received for service: %s, option: %s\n", servicename, optionname);
            
            // Write "No Response" to CSV
            fprintf(csv_file, "\"%s\",0x%03X,\"%s\",\"%s\",\"No Response\",-,\"-\"\n", 
                    timestamp, canid, servicename, optionname);
            fflush(csv_file); // Ensure data is written immediately
        }
    }
}
}