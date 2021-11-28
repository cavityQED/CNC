#ifndef TIMER_H
#define TIMER_H

#include <cstring>
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/timer.h"

#include "esp_err.h"
#include "esp_types.h"

//Motor Driver Connections
#define PULSE_PIN	(gpio_num_t) 15
#define DIR_PIN		(gpio_num_t) 2
#define EN_PIN		(gpio_num_t) 4
#define SYNC_PIN	(gpio_num_t) 26

//Timers
#define LINEAR_GROUP	TIMER_GROUP_0
#define VECTOR_GROUP	TIMER_GROUP_1
#define PULSE_TIMER		TIMER_0
#define	SECONDS_TIMER	TIMER_1

//Base Timer Frequency, 80 MHz
#define BASE_TIMER_FREQUENCY 80000000.0

class Timer {

public:

	enum axis_t 
	{
		X_AXIS = 0x01,
		Y_AXIS = 0x02,
		Z_AXIS = 0x04,
	};

	Timer(int* pos, bool* motion);
	~Timer() {}
	
	void configure_timers();
	void configure_gpio();

	/*	Linear Move Config
	*		Configure a linear move
	*		Pass relevent info to calculate the pulse variables
	*/
	void linear_move_config(	int initial_wait_time_us,
								int final_wait_time_us,
								int accel_steps_per_s_per_s,
								int steps_to_move,
								bool dir);

	/*	Start Linear
	*		Start the linear timer group
	*/
	void start_linear();

	/*	Vector Move Config
	*		Configure a vector move
	*/
	void vector_move_config(	int initial_wait_time_us,
								int final_wait_time_us,
								int accel_steps_per_s_per_s,
								std::shared_ptr<std::vector<bool>> step_vec,
								std::shared_ptr<std::vector<bool>> dirs_vec);

	/*	Start Vector
	*		Start the vector timer group
	*/
	void start_vector();

	/*	RESET
	 * 		Resets the timer counters and enables the interrupts and alarms
	 */
	void reset();

	void set_axis(axis_t a) {m_axis = a;}
	void curve_setup_2D(int x, int y, int xf, int yf, int r, bool d);

	static void IRAM_ATTR G2_2D_circularInterpolation();
	 
	/*	LINEAR PULSE ISR
	*		Pulses PULSE_PIN and updates position_steps when the associated timer's alarm value is reached
	*		Updates the timer's alarm value to vary the motor speed if accelerating
	*		Stops the timer (motor) if cur_step = final_step_count 
	 */
	static void IRAM_ATTR linear_pulse_isr(void* arg);

	/*	VECTOR PULSE ISR
	*		Sets motor direction according to pulse_dirs_vector
	*		Pulses PULSE_PIN according to  pulse_step_vector
	*/
	static void IRAM_ATTR vector_pulse_isr(void* arg);
		
private:
	static std::shared_ptr<std::vector<bool>>	pulse_step_vector;	//Step container used by the Pulse Step ISR
	static std::shared_ptr<std::vector<bool>>	pulse_dirs_vector;	//Direction container used by the Pulse Step ISR
	
	// Pulse Variables
	//		Pulse numbers to indicate motor movement phases
	static int			cur_pulse;			//Current step
	static int			accel_stop_pulse;	//Stop acceleration, begin constant feed rate (divider)
	static int			decel_start_pulse;	//Start decelerating
	static int			final_pulse;		//Stop the motor

	static int			divider_us;			//Divider value for 1 microsecond ticks
	static bool			step_direction;		//Direction of the motor
	static double		divider_tp;			//Constant used to calculate new divider during acceleration phase
	static double		cur_time;			//Store a time in seconds from timer_get_counter_time_sec					
	static double		const_time;			//Stop acceleration phase time
	static double		min_wait_time;		//Minimum wait time in seconds achieved in the pulse timer
	static int*			position_steps;		//Pointer to the motor's step position to be updated in the Pulse Step ISR
	static bool*		in_motion;			//Pointer to the motor's motion bool to be updated by the Pulse Stop ISR

	static axis_t m_axis;
	static axis_t	m_step_axis;
	static bool m_d;
	static int xo, yo;
	static int m_xi, m_yi, m_r;
	static int m_xf, m_yf;
};



#endif
