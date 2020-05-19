/*
 * thingspeak.c
 *
 *  Created on: Nov 27, 2019
 *      Author: Rocío Parra
 */


#include "thingspeak.h"
#if THINGSPEAK_DEBUG != 1
#include "UART/uart.h"
#include "ucosiii_config/os_cfg_app.h"
#endif
#include <stdint.h>


typedef enum {TS_IDLE, TS_WAITKEEPALIVEOK, TS_WAITDATAOK, TS_N_STATES} ts_state_t;
typedef void (*fsm_callback_t)(void);

typedef enum {SEND_DATA, SEND_DATA_OK, SEND_DATA_FAIL, KEEP_ALIVE, KEEP_ALIVE_OK, N_TS_MSGS} ts_msg_t;

#define TS_TIMEOUT_TICKS 	(TS_TIMEOUT_S * OS_CFG_TMR_TASK_RATE_HZ)
#define TS_TIME_MIN_TICKS	(TS_TIME_MIN_S * OS_CFG_TMR_TASK_RATE_HZ)

#define TS_HEADER_LEN	4
#define MAX_RX_MSG_LEN	6

static const unsigned char ts_header[TS_HEADER_LEN] = {0xAA, 0x55, 0xC3, 0x3C};
static const unsigned char ts_lens[N_TS_MSGS] = {
		0x07,	// send data
		0x01, 	// send data ok
		0x01,	// send data fail
		0x01,	// keep alive
		0x01	// keep alive ok
};
static const unsigned char ts_byte_6[N_TS_MSGS] = {
		0x01,	// send data command
		0x81,	// send data ok answer
		0xC1,	// send data fail answer
		0x02,	// keep alive command
		0x82	// keep alive ok answer
};

static OS_TCB ts_tcb;
static CPU_STK ts_stk[TS_STK_SIZE];
static OS_Q q;
static OS_ERR err;

static OS_TMR tmr_timeout;
static OS_TMR tmr_tmin;

//void parse_msg(void);
void no_action(void);
void send_keepalive(void);
void send_data(void);
void restart_timeout(void);
void resend_data(void);

void ts_timeout_callback(void);

ts_state_t next_state[TS_N_STATES][TS_N_EVS] = {
// 							UART				TS_EV_KEEPALIVEOK	TS_EV_DATAOK	TS_EV_DATAFAIL	TS_EV_TIMEOUT			TS_EV_DATA_SENT		TS_EV_DATA
/* IDLE */ 				{ 	TS_IDLE,			TS_IDLE,			TS_IDLE,		TS_IDLE,		TS_IDLE,				TS_WAITDATAOK,		TS_IDLE		},
/* WAITKEEPALIVEOK */	{	TS_WAITKEEPALIVEOK,	TS_IDLE,			TS_IDLE, 		TS_IDLE,		TS_WAITKEEPALIVEOK,		TS_WAITKEEPALIVEOK,	TS_WAITKEEPALIVEOK	},
/* WAITDATAOK */		{	TS_WAITDATAOK,		TS_IDLE,			TS_IDLE,		TS_IDLE,		TS_WAITDATAOK,			TS_WAITDATAOK,		TS_WAITDATAOK		}
};

fsm_callback_t handlers[TS_N_STATES][TS_N_EVS] = {
// 							UART		TS_EV_KEEPALIVEOK	TS_EV_DATAOK	TS_EV_DATAFAIL	TS_EV_TIMEOUT	TS_EV_DATA_SENT	TS_EV_DATA
/* IDLE */ 				{ 	parse_msg,	no_action,			no_action,		no_action,		send_keepalive,	no_action,		send_data	},
/* WAITKEEPALIVEOK */	{	parse_msg,	restart_timeout,	no_action, 		no_action,		no_action,		no_action,		send_data	},
/* WAITDATAOK */		{	parse_msg,	no_action,			no_action,		resend_data,	no_action,		no_action,		send_data	}
};


void ts_keepalive(void);

OS_Q * ts_init(void)
{
	OSQCreate(
		(OS_Q *) &q,
	    (CPU_CHAR *) "Thingspeak queue",
	    (OS_MSG_QTY) TS_MSG_Q_SIZE,
	    (OS_ERR *) &err
	);

	uart_cfg_t uart_cfg = {
		.baudrate = 1200,
		.parity = true,
		.odd_parity = true
	};

	uartInit(0, uart_cfg, &q, TS_EV_UART);	// uart 0


	OSTaskCreate((OS_TCB *)		&ts_tcb,
				 (CPU_CHAR *)	"Thingspeak Task",
				 (OS_TASK_PTR)	ts_run,
				 (void *) 		0u,				// no arguments for ts_run
				 (OS_PRIO) 		TS_TASK_PRIO,
				 (CPU_STK *) 	&ts_stk[0],
				 (CPU_STK_SIZE)	TS_STK_SIZE/10u,
				 (CPU_STK_SIZE)	TS_STK_SIZE,
				 (OS_MSG_QTY) 	0,				// no task msg queue
				 (OS_TICK) 		0u,
				 (void *) 		0u,				// no tcb extension
				 (OS_OPT)		(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
				 (OS_ERR *)		&err
	);

	return &q;
}

void ts_run(void)
{
	OSTmrCreate(
		(OS_TMR *) 				&tmr_timeout,
        (CPU_CHAR *) 			"Thingspeak timeout timer",
		(OS_TICK)               0u, // periodic timer with initial delay equal to the period (default)
		(OS_TICK)               TS_TIMEOUT_TICKS,
		(OS_OPT)                OS_OPT_TMR_PERIODIC,
		(OS_TMR_CALLBACK_PTR)	ts_timeout_callback,
		(void *)				0u, // callback receives no arguments
		(OS_ERR *) 				&err
	);

	OSTmrCreate(
		(OS_TMR *) 				&tmr_tmin,
		(CPU_CHAR *) 			"Thingspeak tmin timer",
		(OS_TICK)               0u, // periodic timer with initial delay equal to the period (default)
		(OS_TICK)               TS_TIMEOUT_TICKS,
		(OS_OPT)                OS_OPT_TMR_PERIODIC,
		(OS_TMR_CALLBACK_PTR)	ts_timeout_callback,
		(void *)				0u, // callback receives no arguments
		(OS_ERR *) 				&err
	);
//  OS_TICK wait_until = OSTimeGet(&os_error);	// the first message can be posted as soon as it arrives
//	OSTmrStart(&tmr_timeout, &os_err);
//
//	while (1) {
//		uint8_t last_msg = OSQPend(
//
//				(OS_TICK)       0u,	// wait forever
//				(OS_OPT)        OS_OPT_PEND_BLOCKING,
//				(OS_MSG_SIZE)	sizeof(uint8_t),
//				(CPU_TS *)      0u, // no timestamp needed
//				(OS_ERR)       	&os_err
//		);
//
//		OSTmrStop(  // i have data to send, don't start keep alive
//			(OS_TMR *) 	&tmr_timeout,
//			(OS_OPT) 	OS_OPT_TMR_NONE,	// don't call the callback
//            (void *) 	0u,					// so no arguments to pass
//            (OS_ERR *)	p_err
//		);
//	}
}


void parse_msg(void)
{
	static uint32_t bytes_received = 0;
	unsigned char buffer[MAX_RX_MSG_LEN+1];

	while (uartIsRxMsg(0)) {
		uint8_t n = uartReadMsg(0, buffer, MAX_RX_MSG_LEN+1);

		for (unsigned int i = 0; i < n; i++) {
			if (bytes_received < TS_HEADER_LEN) {
				bytes_received = buffer[i] == ts_header[bytes_received] ? bytes_received+1 : 0;
			}
			else if (bytes_received == TS_HEADER_LEN) {
				bytes_received = buffer[i] == ts_lens[SEND_DATA_OK] ? bytes_received+1 : 0;
				// keep alive ok and send data ok have the same len
			}
			else if (bytes_received == TS_HEADER_LEN+1) {
                if (buffer[i] == ts_byte_6[SEND_DATA_FAIL]) {
                    OSQPost(
                            (OS_Q *) 		&q,
                            (void *) 		TS_EV_DATAFAIL,
                            (OS_MSG_SIZE)	sizeof(uint8_t),
                            (OS_OPT)		OS_OPT_POST_FIFO,
                            (OS_ERR *) 		err
                    );
                }
			    else if (buffer[i] == ts_byte_6[SEND_DATA_OK]) {
					OSQPost(
						(OS_Q *) 		&q,
						(void *) 		TS_EV_DATAOK,
						(OS_MSG_SIZE)	sizeof(uint8_t),
						(OS_OPT)		OS_OPT_POST_FIFO,
						(OS_ERR *) 		err
					);
				}
				else if (buffer[i] == ts_byte_6[KEEP_ALIVE_OK]) {
					OSQPost(
						(OS_Q *) 		&q,
						(void *) 		TS_EV_KEEPALIVEOK,
						(OS_MSG_SIZE)	sizeof(uint8_t),
						(OS_OPT)		OS_OPT_POST_FIFO,
						(OS_ERR *) 		err
					);
				}

				bytes_received = 0;
			}
		}
	}
}


void no_action(void)
{
	;
}
void send_keepalive(void){;}
void send_data(void){;}

void restart_timeout(void){;}
void resend_data(void){;}

void ts_timeout_callback(void){;}
