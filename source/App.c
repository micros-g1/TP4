/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <rtos/uCOSIII/src/uCOS-III/Source/os.h>
//#include "Display/display_interface.h"
//#include "Rotary_encoder/rotary_encoder.h"
//#include "Database/database.h"
//#include "Magnetic_stripe/magnetic_stripe.h"
//#include "fsm.h"
//#include "events.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
//void m_finished(void);
//void rotary_encoder_callback(re_event_t ev);
//void timeout_callback();
//extern void ms_callback(ms_ev_t ev);

//fsm_state_t * state;
//fsm_event_t event;

OS_TMR * fsm_timer;
char fsm_timer_name[] = "fsm_timer";
OS_ERR * fsm_timer_err;
OS_TICK fsm_timer_tick_period;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/* Task TP1 */
void TP1_task (void){


//	rotary_encoder_init();
//	rotary_encoder_set_callback(rotary_encoder_callback);
//
//	display_init_interface(m_finished);
//	u_init();
//	ms_init(ms_callback);
//
//	display_write_to_led(1, true);
//	display_write_to_led(0, true);
//	display_write_to_led(2, true);
//
//	init_event_queue();
//	state = fsm_get_init_state();
//
//	uint32_t delay_in_seconds = 10;
//
//	fsm_timer_tick_period = delay_in_seconds * OSCfg_TickRate_Hz;
//	OSTmrCreate(fsm_timer, fsm_timer_name, 0, fsm_timer_tick_period, OS_OPT_TMR_PERIODIC, timeout_callback, NULL, fsm_timer_err);

	while(1){
		//directiva de sistema operativo.
//		if(is_there_event()){
//			OSTmrStart(fsm_timer, fsm_timer_err);
//			pop_event(&event);
//			state = fsm_run(state, event);
//		}
//
	}

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
//void rotary_encoder_callback(re_event_t ev){
//	fsm_event_t new_event;
//	bool ev_flag = false;
//	switch(ev)
//	{
//		case RE_LEFT:
//			new_event.code = DOWN_EV;
//			ev_flag = true;
//			break;
//		case RE_RIGHT:
//			new_event.code = UP_EV;
//			ev_flag = true;
//			break;
//		case RE_KEY_LEFT:
//			ev_flag = false;
//			new_event.code = BRIGHT_DOWN_EV;
//			display_set_brightness_up_down(DISPLAY_INT_DOWN);
//			break;
//		case RE_KEY_RIGHT:
//			ev_flag = false;
//			new_event.code = BRIGHT_UP_EV;
//			display_set_brightness_up_down(DISPLAY_INT_UP);
//			break;
//		case RE_SHORT_CLICK:
//			ev_flag = true;
//			new_event.code = ENTER_EV;
//			break;
//		case RE_LONG_CLICK_1:
//			ev_flag = true;
//			new_event.code = BACK_EV;
//			break;
//		case RE_LONG_CLICK_2:
//			ev_flag = true;
//			new_event.code = CANCEL_EV;
//			break;
//		default:
//			new_event.code = NO_EV;
//			ev_flag = true;
//			break;
//	}
//	if(new_event.code != NO_EV && ev_flag)
//		push_event(new_event);
//}
//
//void m_finished(){
//	fsm_event_t ev;
//	ev.code = MARQUEE_END_EV;
//	push_event(ev);
//}
//
//void timeout_callback(){
//	fsm_event_t ev;
//	ev.code = TIMEOUT_EV;
//	push_event(ev);
//}

/*******************************************************************************
 ******************************************************************************/
