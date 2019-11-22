/*
 * rotary_encoder.c
 *
 *  Created on: 3 Sep 2019
 *      Author: grein
 */

#include <gpio.h>
#include <PIT/pit.h>
#include <Rotary_encoder/rotary_encoder.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"

//TODO: Replace Systick with PIT. Determine PIT time.
//This driver has an ISR called by PIT
//ISR Call Frequency
#define RE_ISR_FREQUENCY	1000UL
//Number of PIT clock ticks per ISR CALL
#define RE_ISR_PERIOD_TICKS PIT_CLOCK_FREQUENCY/RE_ISR_FREQUENCY - 1
//Callback
static void rotary_encoder_ISR();

//The (low level) events generated by the rotary encoder and timer are: (DO NOT MODIFY ORDER OF ENUM)
typedef enum {RE_LL_LEFT,RE_LL_RIGHT,RE_LL_BUTTON_DOWN,RE_LL_BUTTON_UP,RE_LL_TIMEOUT,RE_LL_NO_EVENT,RE_LL_TOTAL_EVENTS} re_ll_event_t;

//Rotary Encoder Rotation Behavior:
//
//		------> (CW)
//       =  =           					|^	-----             --------
//    =        =       			SIGNAL A	|		|             |
//   =          =     						|.		---------------
//   =          =    						|^	-------             ------
//    =        =       			SIGNAL B	|	  	  |             |
//       =  =          						|.		  ---------------
//
//		<------ (CCW)
//       =  =           					|^	-------             ------
//    =        =       			SIGNAL A	|	  	  |             |
//   =          =     						|.		  ---------------
//   =          =    						|^	-----             --------
//    =        =       			SIGNAL B	|		|             |
//       =  =          						|.		---------------
//

//Analyzing the falling edges only, this behavior can be modeled as
static const re_ll_event_t rotary_encoder_rotation_decode[2][2] =
{				//If previous state was A=1, B=1  (rotation idle), and now:
//Signal A:		--------------[0]------------- 		--------------[1]-------------
//Signal B:		------[0]-----//------[1]-----		------[0]-----//------[1]-----
//Then event is:
				{RE_LL_NO_EVENT,	RE_LL_LEFT},	{RE_LL_RIGHT,	RE_LL_NO_EVENT}
};

//The Button case is simpler, pulled-up N.O. button. Behavior can be modeled as
static const re_ll_event_t rotary_encoder_button_decode[2][2] =
{
//Button Signal was		--------------[0]------------- 		--------------[1]-------------
//Button Signal is now	------[0]-----//------[1]-----		------[0]-----//------[1]-----
//Then event is:
						{RE_LL_NO_EVENT,RE_LL_BUTTON_UP},	{RE_LL_BUTTON_DOWN,	RE_LL_NO_EVENT}
};

//Rotary Encoder can be in any of the following states: (DO NOT MODIFY ORDER OF ENUM)
typedef enum {RE_IDLE_S,RE_PRESS_1_S,RE_PRESS_2_S,RE_PRESS_3_S,RE_KEYMODE_S,RE_TOTAL_STATES} re_state_t;
//RE_IDLE_S: Rotary Encoder waiting for input
//RE_PRESS_1_S:	Rotary Encoder pressed down
//RE_PRESS_2_S:	Rotary Encoder pressed down, T <= down time < 2T
//RE_PRESS_3_S: Rotary Encoder pressed down, down time >= 2T
//RE_KEYMODE_S:	Rotary Encoder pressed down, while down, rotation before time T

//Time T -> Timer will generate event RE_LL_TIMEOUT
//Timeout will be generated after RE_TIMEOUT_ISR_TICKS executions of the ISR
#define RE_TIMEOUT_ISR_TICKS	500

//3 Sources of (low-level) events: "Button","Rotation","Timeout"
//Any (non RE_LL_NO_EVENT) event from "Button" or "Rotation" will override a "Timeout" event
//If event from "Button" is generated, "Rotation" will not be checked for events
//(Simultaneous event from "Button" and "Rotation" -> Rotation event discarded).

//Rotary Encoder (low level) events will change Rotary Encoder state according to Rotary Encoder FSM:
static const re_state_t  rotary_encoder_fsm_next_state[RE_TOTAL_STATES][RE_LL_TOTAL_EVENTS] =
{
						//RE_LL_LEFT		//RE_LL_RIGHT		//RE_LL_BUTTON_DOWN	//RE_LL_BUTTON_UP	//RE_LL_TIMEOUT		//RE_LL_NO_EVENT
/*RE_IDLE_S*/			{RE_IDLE_S,			RE_IDLE_S,			RE_PRESS_1_S,		RE_IDLE_S,			RE_IDLE_S,			RE_IDLE_S		},
/*RE_PRESS_1_S*/		{RE_KEYMODE_S,		RE_KEYMODE_S,		RE_PRESS_1_S,		RE_IDLE_S,			RE_PRESS_2_S,		RE_PRESS_1_S	},
/*RE_PRESS_2_S*/		{RE_PRESS_2_S,		RE_PRESS_2_S,		RE_PRESS_2_S,		RE_IDLE_S,			RE_PRESS_3_S,		RE_PRESS_2_S	},
/*RE_PRESS_3_S*/		{RE_PRESS_3_S,		RE_PRESS_3_S,		RE_PRESS_3_S,		RE_IDLE_S,			RE_PRESS_3_S,		RE_PRESS_3_S	},
/*RE_KEYMODE_S*/		{RE_KEYMODE_S,		RE_KEYMODE_S,		RE_KEYMODE_S,		RE_IDLE_S,			RE_KEYMODE_S,		RE_KEYMODE_S	}
};

//Each state transition will generate a (high level) Rotary Encoder event according to:
static const re_event_t  rotary_encoder_generated_event[RE_TOTAL_STATES][RE_LL_TOTAL_EVENTS] =
{
						//RE_LL_LEFT	//RE_LL_RIGHT	//RE_LL_BUTTON_DOWN	//RE_LL_BUTTON_UP	//RE_LL_TIMEOUT		//RE_LL_NO_EVENT
/*RE_IDLE_S*/			{RE_LEFT,		RE_RIGHT,		RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT},
/*RE_PRESS_1_S*/		{RE_KEY_LEFT,	RE_KEY_RIGHT,	RE_NO_EVENT,		RE_SHORT_CLICK,		RE_LONG_CLICK_1,	RE_NO_EVENT},
/*RE_PRESS_2_S*/		{RE_NO_EVENT,	RE_NO_EVENT,	RE_NO_EVENT,		RE_NO_EVENT,		RE_LONG_CLICK_2,	RE_NO_EVENT},
/*RE_PRESS_3_S*/		{RE_NO_EVENT,	RE_NO_EVENT,	RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT},
/*RE_KEYMODE_S*/		{RE_KEY_LEFT,	RE_KEY_RIGHT,	RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT,		RE_NO_EVENT}
};

//Variables
static rotary_encoder_callback_t rotary_encoder_callback;
static bool previous_button_signal;
static bool previous_rotation_idle;
static re_state_t re_state;

//Initialize Rotary Encoder
void rotary_encoder_init()
{
	static bool rotary_encoder_initialized = false;
	if(!rotary_encoder_initialized)
	{
		//Initialize GPIO
		gpioMode (RE_PIN_BUTTON_SIGNAL, INPUT_PULLUP);
		gpioMode (RE_PIN_SIGNAL_A, INPUT_PULLUP);
		gpioMode (RE_PIN_SIGNAL_B, INPUT_PULLUP);
		//Variable Initialization
		rotary_encoder_callback = NULL;
		previous_button_signal = true;	//Assume button is not pressed
		re_state = RE_IDLE_S;		//Assume rotary encoder rotation is IDLE
		previous_rotation_idle = true;
		//Initialize PIT
		pit_init();
		pit_conf_t pit_conf = {
				.callback=rotary_encoder_ISR,
				.chain_mode=false,
				.channel=PIT_ROTARY_CH,
				.timer_count=RE_ISR_PERIOD_TICKS,
				.timer_enable=true,
				.timer_interrupt_enable=true
		};
		pit_set_channel_conf(pit_conf);
		//Done.
		rotary_encoder_initialized = true;
	}
}

//Set Rotary Encoder Callback
void rotary_encoder_set_callback(rotary_encoder_callback_t callback)
{
	rotary_encoder_callback = callback;
}

static void rotary_encoder_ISR()
{
	static unsigned int tick_counter = 0;
	//Low Level Event
	re_ll_event_t ev_ll = RE_LL_NO_EVENT;
	//High Level Event (for callback)
	re_event_t ev = RE_NO_EVENT;

	//Measured Signals
	bool current_button_signal = gpioRead(RE_PIN_BUTTON_SIGNAL);
	bool current_A_signal = gpioRead(RE_PIN_SIGNAL_A);
	bool current_B_signal = gpioRead(RE_PIN_SIGNAL_B);
	bool current_rotation_idle = current_A_signal && current_B_signal;
	//Get (low level) event from button
	ev_ll = rotary_encoder_button_decode[previous_button_signal][current_button_signal];
	//Get (low level) event from rotation, if no event from button && rotation is idle
	if(ev_ll == RE_LL_NO_EVENT && previous_rotation_idle)
		ev_ll = rotary_encoder_rotation_decode[current_A_signal][current_B_signal];
	//Get (low level) event from timer, if no event from button nor rotation
	if(ev_ll == RE_LL_NO_EVENT)
	{
		if(tick_counter == RE_TIMEOUT_ISR_TICKS)
		{
			ev_ll = RE_LL_TIMEOUT;
			//Timeout event, reset tick counter
			tick_counter = 0;
		}
		else
			//Increment counter
			tick_counter++;
	}
	else
		//Non Timer event causes timer to restart
		tick_counter = 0;
	//ev_ll is a low level event for the FSM. Generate high level event
	ev = rotary_encoder_generated_event[re_state][ev_ll];
	if(ev != RE_NO_EVENT)
		rotary_encoder_callback(ev);
	//Update All Variables
	re_state = rotary_encoder_fsm_next_state[re_state][ev_ll];
	previous_button_signal = current_button_signal;
	previous_rotation_idle = current_rotation_idle;

}
