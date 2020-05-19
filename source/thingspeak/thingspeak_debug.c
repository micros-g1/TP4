//
// Created by Rocío Parra on 11/28/2019.
//

#include "thingspeak_debug.h"



void  OSQCreate (OS_Q        *p_q,
                 CPU_CHAR    *p_name,
                 OS_MSG_QTY   max_qty,
                 OS_ERR      *p_err)
{
    q_init(p_q);
}


void  OSQPost (OS_Q         *p_q,
               void         *p_void,
               OS_MSG_SIZE   msg_size,
               OS_OPT        opt,
               OS_ERR       *p_err)
{
    q_pushback(p_q, (uint8_t)p_void);
}

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
                    OS_ERR        *p_err)
{
    ;
}


void  OSTmrCreate (OS_TMR               *p_tmr,
                   CPU_CHAR             *p_name,
                   OS_TICK               dly,
                   OS_TICK               period,
                   OS_OPT                opt,
                   OS_TMR_CALLBACK_PTR   p_callback,
                   void                 *p_callback_arg,
                   OS_ERR               *p_err)
{
    p_tmr->periodic = opt != OS_OPT_TMR_ONE_SHOT;
    p_tmr->reload = p_tmr->periodic ? period : dly;
    p_tmr->running = false;
    p_tmr->callback = p_callback;
}


static queue_t uart_q;
void uartInit (uint8_t id, uart_cfg_t config, OS_Q * queue, uint8_t msg_to_post)
{
    q_init(&uart_q);
}

bool uartIsRxMsg(uint8_t id)
{
    return uart_q.len > 0;
}

uint8_t uartReadMsg(uint8_t id, uint8_t * msg, uint8_t cant)
{
    uint8_t i = 0;
    while (i < cant && uart_q.len) {
        msg[i++] = q_popfront(&uart_q);
    }

    return i;
}

uint8_t uartWriteMsg(uint8_t id, const uint8_t* msg, uint8_t cant)
{
    for (unsigned int i = 0; i < cant; i++)
    {
        q_pushback(&uart_q, msg[i]);
    }
}