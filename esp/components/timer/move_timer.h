#ifndef TIMER_H
#define TIMER_H

#include <cstring>
#include <vector>
#include <iostream>

#include "driver/gpio.h"
#include "driver/timer.h"

#include "esp_err.h"
#include "esp_types.h"

//Motor Driver Connections
#define STEP_PIN	(gpio_num_t) 12
#define DIR_PIN		(gpio_num_t) 14
#define EN_PIN		(gpio_num_t) 27

//Timers
#define PULSE_GROUP	TIMER_GROUP_0
#define STEP_TIMER	TIMER_0
#define STOP_TIMER	TIMER_1

//Final Timer Tick Period
#define FINAL_TICK_PERIOD .000001

//Base Timer Frequency, 80 MHz
#define BASE_TIMER_FREQUENCY 80000000

namespace timer {
	void setup();	//Configures the timers and gpio pins
	void configure_timers();
	void configure_gpio();
	
	/*	SETUP MOVE
	 * 		Sets timer alarm values and variable values used by timer callbacks
	 * 	@param step			- container of step commands
	 * 	@param dirs			- container of directions for the steps
	 * 	@param wait_time_us	- wait time in microseconds between timer ticks
	 * 	@param pos			- pointer to motor position
	 * 	@param motion		- pointer to motor motion bool
	 */
	void setup_move(	std::vector<bool> step, 
						std::vector<bool> dirs, 
						int wait_time_us,
						int* pos,
						bool* motion);
	
	/*	START MOVE
	 * 		Starts the timers
	 */
	void start_move();
	
	/*	RESET
	 * 		Resets the timer counters and enables interrupts and alarms
	 */
	void reset();
		
	//Timer ISR Callbacks
	void IRAM_ATTR pulse_step_isr(void* arg);
	void IRAM_ATTR pulse_stop_isr(void* arg);
		
	
	//Step and dir vectors for move operations
	static std::vector<bool> pulse_step_vector{0};
	static std::vector<bool> pulse_dirs_vector{0};
	
	static int		cur_pulse = 0;	//Current pulse number
	static int*		position_steps;	//Pointer to motor position
	static bool		direction = 1;	//Motor direction
	static bool*	in_motion;		//Motor motion
		
}//timer namespace

#endif
