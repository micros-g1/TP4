/*
 * App.h
 *
 *  Created on: 22 nov. 2019
 *      Author: Tomas
 */

#ifndef APP_H_
#define APP_H_

/* App_Run */
#define TP1_TASK_STK_SIZE 		512u
#define TP1_TASK_PRIO 			2u
static OS_TCB TP1_TASK_TCB;
static CPU_STK Tp1_TaskStk[TASKSTART_STK_SIZE];

void Tp1_Task (void);

#endif /* APP_H_ */
