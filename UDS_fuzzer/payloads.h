#ifndef PAYLOADS_H
#define PAYLOADS_H

#include "all_headers.h"

/*================================================================
payloads.h
'payloads' structure define
every CAN packets and payload definition here, declared in payloads.c
=================================================================*/

/*================================================================
struct payloads
=================================================================*/
struct payloads {
    unsigned char payload1[8];
    unsigned char payload2[8];
    unsigned char payload3[8];
    unsigned char payload4[8];
    unsigned char payload5[8];
    size_t payload1_len;
    size_t payload2_len;
    size_t payload3_len;
    size_t payload4_len;
    size_t payload5_len;
};

/*================================================================
struct answer
=================================================================*/
struct answer {
    const char *answer10;
    const char *answer11;
    const char *answer12;
    const char *answer13;
    const char *answer21;
    const char *answer22;
    const char *answer24;
    const char *answer31;
    const char *answer33;
    const char *answer50;
    const char *answer51;
    const char *answer67;
    const char *answerC5;
    const char *answer7E;
};
/*================================================================
struct csv_report
=================================================================*/
struct csv_report {
    int canid;
    char servicename[60];
    int sent_payload;
    char reply_comment[45];
};

/*================================================================
struct uds_request
=================================================================*/
struct uds_request {
    unsigned char payload1[8];
    unsigned char payload2[8];
    unsigned char payload3[8];
    unsigned char payload4[8];
    unsigned char payload5[8];
    size_t payload1_len;
    size_t payload2_len;
    size_t payload3_len;
    size_t payload4_len;
    size_t payload5_len;
    const char *service1;
    const char *service2;
    const char *service3;
    const char *service4;
    const char *service5;
};

/*================================================================
extern declarations for payloads
=================================================================*/
extern struct payloads msq;             // Masquerade, masqueradeCheck
extern struct payloads fuzzing;         // findUDS, findBUDS, randomCANID, randomPayload
extern struct payloads dos;             // DOS
extern struct payloads replay;          // Replay, suddenaccel
extern struct payloads suspension;      // resetecu, dos(?)

/*================================================================
extern declarations for uds_reply and uds_request instances
=================================================================*/
extern struct answer uds_reply;

// UDS Service Requests
extern struct uds_request diagnostic_session_control;         // Service 0x10 (DIAGNOSTIC SESSION CONTROL)
extern struct uds_request ECU_reset;                          // Service 0x11 (ECU RESET)
extern struct uds_request clear_diagnostic_information;       // Service 0x14 (CLEAR DIAGNOSTIC INFORMATION)
extern struct uds_request read_dtc_information;               // Service 0x19 (READ DTC INFORMATION)
extern struct uds_request read_data_by_identifier;            // Service 0x22 (READ DATA BY IDENTIFIER)
extern struct uds_request read_memory_by_address;             // Service 0x23 (READ MEMORY BY ADDRESS)
extern struct uds_request security_access;                    // Service 0x27 (SECURITY ACCESS)
extern struct uds_request write_data_by_identifier;           // Service 0x2E (WRITE DATA BY IDENTIFIER)
extern struct uds_request routine_control;                    // Service 0x31 (ROUTINE CONTROL)
extern struct uds_request request_download;                   // Service 0x34 (REQUEST DOWNLOAD)
extern struct uds_request request_upload;                     // Service 0x35 (REQUEST UPLOAD)
extern struct uds_request transfer_data;                      // Service 0x36 (TRANSFER DATA)
extern struct uds_request request_transfer_exit;              // Service 0x37 (REQUEST TRANSFER EXIT)
extern struct uds_request write_memory_by_address_3D;        // Service 0x3D (WRITE MEMORY BY ADDRESS)
extern struct uds_request tester_present;                     // Service 0x3E (TESTER PRESENT)
extern struct uds_request control_dtc_settings;              // Service 0x85 (CONTROL DTC SETTINGS)
extern struct uds_request link_control;                       // Service 0x87 (LINK CONTROL)

#endif // PAYLOADS_H