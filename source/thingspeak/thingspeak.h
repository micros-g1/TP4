/*
 * thingspeak.h
 *
 *  Created on: Nov 27, 2019
 *      Author: Rocío Parra
 */

#ifndef THINGSPEAK_THINGSPEAK_H_
#define THINGSPEAK_THINGSPEAK_H_

#define THINGSPEAK_DEBUG    0

#if THINGSPEAK_DEBUG != 1
#include <rtos/uCOSIII/src/uCOS-III/Source/os.h>
#else
#include "thingspeak_debug.h"
#endif

#define TS_STK_SIZE		1024
#define TS_TASK_PRIO	5
#define TS_MSG_Q_SIZE	10

#define TS_TIME_MIN_S	16
#define TS_TIMEOUT_S	30


typedef enum {TS_EV_UART, TS_EV_KEEPALIVEOK, TS_EV_DATAOK, TS_EV_DATAFAIL, TS_EV_TIMEOUT, TS_EV_MINTIMEELAPSED, TS_EV_DATA, TS_N_EVS} ts_ev_t;


OS_Q * ts_init(void);

void ts_run(void);

void parse_msg(void);

#endif /* THINGSPEAK_THINGSPEAK_H_ */
