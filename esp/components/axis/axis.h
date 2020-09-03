#ifndef AXIS_H
#define AXIS_H

#include <cmath>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp32/rom/ets_sys.h"

#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_timer.h"
#include "esp_types.h"

#include "spi_client.h"

//GPIO numbers motor driver connections
#define STEP	(gpio_num_t) 12
#define DIR		(gpio_num_t) 14
#define EN		(gpio_num_t) 27
#define SYNC	(gpio_num_t) 26

//Timers
#define LINE_GROUP	TIMER_GROUP_0
#define CURV_GROUP	TIMER_GROUP_1
#define STEP_TIMER	TIMER_0
#define STOP_TIMER	TIMER_1

//Timer period in microseconds
//Used to set timer divider
#define PERIOD_uS	1

class axis {
public:
	/*	Constructor
	 * 		sets up spi lines and stepper motor driver lines	
	 */
	axis();
	
	/*	Destructor	*/
	~axis() {}
	
	/*	Check error function; prints the input and then the error	*/
	void check_error(const char* msg);
	
	/*	Functions called by the constructor to setup the 
	 * 		various timers, spi, etc	
	 */
	void setup_timers();
	void setup_gpio();
	void set_defaults();	
	
	/*	Set the SPI Pointer	*/
	void set_spi(SpiClient *s) {spi = s;}
	
	/*	Setters for stepper motor parameters
	 * 		that are constant over the entire life of the axis
	 * 		different from parameters such as speed, steps_to_move,
	 * 		etc. that change from operation to operation	
	 */
	void set_steps_per_mm(int spmm) {steps_per_mm = spmm;}
	void set_max_travel_steps(int max_steps) {max_travel_steps = max_steps;}
	void set_backlash(int b) {backlash = b;}
	void set_x_axis(bool x) {x_axis = x;}
	
	/*	Setters for operation specific parameters
	 * 		like speed, steps_to_move, etc	
	 */
	void set_feed_rate(int speed);
	void set_step_time(int time);
	void set_steps_to_move(int steps);
	void set_direction(bool dir);
	void set_jog_steps(int steps);
		
	/*	Printers	*/
	void print_pos_steps() {
		std::cout << "Position: " << position_steps << '\n';
	}
	
	/*	Motor moving and helper functions	*/
	void move();
	void move_jog_mode();
	void move_line_mode();
	void move_curv_mode();
	
	void stop();
	void stop_zero_interlock();
	void find_zero();
	
	void reset_timer_counters();
	void setup_curve(std::vector<int> &info);
	
	/*	Getters	*/
	int get_position_steps() {return position_steps;}
	int in_motion() {return (int)motor_in_motion;}
	
	/*	Set the move mode	*/
	void enable_jog_mode(bool enable);
	void enable_line_mode(bool enable);
	void enable_curv_mode(bool enable);
	void enable_sync_mode(bool enable);
	
	/* 	Timer Callbacks	*/
	static void IRAM_ATTR line_step_isr(void* arg);
	static void IRAM_ATTR line_stop_isr(void* arg);
	static void IRAM_ATTR curv_step_isr(void* arg);
	static void IRAM_ATTR curv_stop_isr(void* arg);
	
	/* Sync Semaphore Release Interrupt	*/
	static void IRAM_ATTR syncSem_release_isr(void* arg);
			 
private:				
	//Motor Variables
	int steps_per_mm;						//Motor steps per mm
	int max_travel_steps;					//Max motor travel in steps
	int	backlash;							//Number of extra steps needed to move on direction change
	bool x_axis		= false;				//True if this is the x_axis
	
	int	feed_rate;							//Feed rate in mm/s
	int pulse_period_us;					//Wait time between steps in microseconds
	int steps_to_move;						//Steps to move
		
	static std::vector<bool> step_vec;		//Holds true for step, false for no step during a curve move
	static std::vector<bool> dirs_vec;		//Direction of each step during a curve move
	static int step_num;					//Step number for accessing curve vectors
	static const int zero_steps = 200;		//Number of steps to move once zero position is found
	static const int jog_wait_time = 250;	//Time between steps in microseconds during jog
		
	//Jog Mode Variables
	int jog_steps = 0;					//Number of steps to move in one jog step
	
	//Position and Direction Variables
	bool zeroing = false;					//True if motor is trying to find machine zero
	static bool motor_in_motion;			//True when the motor is in motion
	static bool motor_direction;			//Direction of motor
	static int	position_steps;				//Position of the motor in steps from zero

	//Move Modes
	bool jog_mode = false;
	bool line_mode = false;
	bool curv_mode = false;
	bool sync_mode = false;

	//ESP Error
	esp_err_t error = ESP_OK;
			
	//Semaphore for Sync Move
	static xQueueHandle syncSem;
	
	//SPI Client to Toggle Ready Pin
	SpiClient *spi;
};

#endif	
