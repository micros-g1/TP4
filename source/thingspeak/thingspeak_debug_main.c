//
// Created by Rocío Parra on 11/28/2019.
//

#include "thingspeak.h"
#include "thingspeak_debug.h"
#include <stdlib.h>


#define MSG_LEN 6
#define MSG_QTY 3

int main (void);

int main(void)
{
    OS_Q * q = ts_init();

    unsigned char send_data_ok[MSG_LEN] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0x81};
    unsigned char send_data_fail[MSG_LEN] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0xC1};
    unsigned char keep_alive_ok[MSG_LEN] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0x82};

    unsigned char * msgs[MSG_QTY] = {send_data_ok, send_data_fail, keep_alive_ok};

    for (unsigned int i = 0; i < MSG_QTY; i++) {
        uartWriteMsg(0, msgs[i], MSG_LEN);
        parse_msg();
    }

    return EXIT_SUCCESS;
}