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

class Timer {
public:
	Timer();
	~Timer() {}
	
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
	void move_setup(	const std::vector<bool> &step, 
						const std::vector<bool*> &dirs, 
						int wait_time_us,
						int* pos,
						bool* motion,
						bool accel = true);
	/*	START MOVE
	 * 		Starts the timers
	 */
	void start_move();
	
	/*	RESET
	 * 		Resets the timer counters and enables the interrupts and alarms
	 */
	void reset();
	 
	/*	PULSE STEP ISR
	 * 		Executes when the Step Timer counter reached the alarm value.
	 * 		First sends a step signal to the motor driver if the step vector is
	 * 			true at the current pulse position.
	 * 		Then updates the position.
	 * 		Then increments the cur_pulse counter (if in accel mode)
	 * 			and sets the direction for the next step
	 */
	static void IRAM_ATTR pulse_step_isr(void* arg);
	
	/*	PULSE STOP ISR
	 * 		Stops the timer counters at the end of the move.
	 * 		Sets the motion bool to false.
	 */
	static void IRAM_ATTR pulse_stop_isr(void* arg);
	
private:
	static std::vector<bool>	pulse_step_vector;	//Step container used by the Pulse Step ISR
	static std::vector<bool*>	pulse_dirs_vector;	//Direction container used by the Pulse Step ISR
	
	static int		cur_pulse;		//Pulse counter
	static int*		position_steps;	//Pointer to the motor's step position to be updated in the Pulse Step ISR
	static bool		direction;		//Direction of the motor
	static bool		accel_mode;		//True if Pulse Step ISR should increment cur_pulse, false otherwise
	static bool*	in_motion;		//Pointer to the motor's motion bool to be updated by the Pulse Stop ISR
};

#endif
