//
// Created by Rocío Parra on 11/28/2019.
//

#ifndef SOURCE_THINGSPEAK_DEBUG_H
#define SOURCE_THINGSPEAK_DEBUG_H

#include "Util/queue.h"
#include <time.h>
#include <stdbool.h>

typedef queue_t OS_Q;
typedef void * OS_TCB;
typedef void * CPU_STK;
typedef void * OS_ERR;
typedef uint32_t OS_MSG_QTY;
typedef uint32_t OS_MSG_SIZE;

typedef char CPU_CHAR;
typedef uint32_t OS_TICK;

typedef void * OS_TASK_PTR;
typedef uint32_t OS_PRIO;
typedef uint32_t CPU_STK_SIZE;

typedef void (* OS_TMR_CALLBACK_PTR)(void);
typedef struct {
    bool periodic;
    bool running;
    clock_t reload;
    clock_t t_init;
    clock_t t_expires;
    OS_TMR_CALLBACK_PTR callback;
} OS_TMR;


typedef enum {
    OS_OPT_TASK_STK_CHK, OS_OPT_TASK_STK_CLR, OS_OPT_TASK_SAVE_FP,
    OS_OPT_POST_FIFO,
    OS_OPT_PEND_BLOCKING,
    OS_OPT_TMR_NONE,
    OS_OPT_TMR_PERIODIC, OS_OPT_TMR_ONE_SHOT
} OS_OPT;

#define OS_CFG_TMR_TASK_RATE_HZ 100



void  OSQCreate (OS_Q        *p_q,
                 CPU_CHAR    *p_name,
                 OS_MSG_QTY   max_qty,
                 OS_ERR      *p_err);

void  OSQPost (OS_Q         *p_q,
               void         *p_void,
               OS_MSG_SIZE   msg_size,
               OS_OPT        opt,
               OS_ERR       *p_err);

void  OSTaskCreate (OS_TCB        *p_tcb,
                    CPU_CHAR      *p_name,
                    OS_TASK_PTR    p_task,
                    void          *p_arg,
                    OS_PRIO        prio,
                    CPU_STK       *p_stk_base,
                    CPU_STK_SIZE   stk_limit,
                    CPU_STK_SIZE   stk_size,
                    OS_MSG_QTY     q_size,
                    OS_TICK        time_quanta,
                    void          *p_ext,
                    OS_OPT         opt,
                    OS_ERR        *p_err);

void  OSTmrCreate (OS_TMR               *p_tmr,
                   CPU_CHAR             *p_name,
                   OS_TICK               dly,
                   OS_TICK               period,
                   OS_OPT                opt,
                   OS_TMR_CALLBACK_PTR   p_callback,
                   void                 *p_callback_arg,
                   OS_ERR               *p_err);

typedef struct {
    uint8_t parity : 1; 		// true for using parity bit
    uint8_t odd_parity : 1;		// true for odd parity (if parity is true)
    uint32_t baudrate;
} uart_cfg_t;


/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, word size)
*/
void uartInit (uint8_t id, uart_cfg_t config, OS_Q * queue, uint8_t msg_to_post);

/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has being received
*/
bool uartIsRxMsg(uint8_t id);

uint8_t uartReadMsg(uint8_t id, uint8_t * msg, uint8_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transferred
 * @param cant Desired quantity of bytes to be transferred
 * @return Real quantity of bytes to be transferred
*/
uint8_t uartWriteMsg(uint8_t id, const uint8_t* msg, uint8_t cant);


#endif //SOURCE_THINGSPEAK_DEBUG_H
