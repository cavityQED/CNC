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
#define PERIODIC	TIMER_GROUP_0
#define ONE_SHOT	TIMER_GROUP_1
#define PULSE		TIMER_0
#define ACCEL		TIMER_1
//Timer period in microseconds
//Used to set timer divider
#define PERIOD_uS	1

//Function codes for spi transmission
enum AXIS_FUNCTION_CODE {
	SET_SPEED, 			
	SET_DIRECTION,
	SET_STEPS_TO_MOVE,
	SET_MM_TO_MOVE,
	SET_JOG_SPEED_STEPS,
	SET_JOG_SPEED_MM,
	ENA_JOG_MODE,		
	DIS_JOG_MODE,		
	ENA_STEP_MODE,
	DIS_STEP_MODE,
	ENA_SYNC_MODE,
	DIS_SYNC_MODE,	
	GET_POSITION, 		
	SET_MM_PER_STEP,
	SET_STEPS_PER_REVOLUTION,
	ZERO,
	ZERO_INTERLOCK_STOP,
	MOVE_TO,	
	MOVE,
	CIRCLE_MOVE,
	STOP,
	RECEIVE,
	SETUP_CIRCLE_MOVE,
	SET_CIRCLE_OPS,
	GET_CIRCLE_STEPS,
	GET_CIRCLE_TIMES,
	GET_CIRCLE_DIRS,
	PRINT_CIRCLE_INFO,
	MOTOR_READY,
	MOTOR_NOT_READY
};

//Struct to hold timer callback arguments
typedef struct {
	uint64_t pulseAlarmTimes[10];	//Times to update the pulse alarm
	int accelStep;					//What step of the accel the timer is on
} timer_args_t;

//Struct to hold data about a circular move
typedef struct {
	int					num_ops;	//Number of operations
	std::vector<int>	op_steps;	//Step counts for each operation
	std::vector<int>	op_times;	//Wait times between steps in each operation
	std::vector<int>	op_dirs;	//Direction for each operation
} circle_info_t;

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
	void set_mm_per_step(double mmps);
	void set_mm_per_step(int mm, int ten_power);	//Set mm per step by passing the mm in an int and the power of ten to divide it by
	void set_max_travel_steps(int max_steps);
	void set_steps_per_revolution(int steps);		//Set the steps per revolution; depends on the microstepping
	
	/*	Setters for operation specific parameters
	 * 		like speed, steps_to_move, etc	
	 */
	void set_speed_rpm(int speed);
	void set_direction(bool dir);
	void set_steps_to_move(int steps);
	void set_mm_to_move(int whole, int dec);
	
	/*	Getters and Setters for circle move info	*/
	void set_circle_ops(int ops) {
		circle_info.num_ops = ops;
		circle_info.op_steps.resize(ops);
		circle_info.op_times.resize(ops);
		circle_info.op_dirs.resize(ops);
	}
	std::vector<int>& get_circle_steps() 	{return circle_info.op_steps;}
	std::vector<int>& get_circle_times() 	{return circle_info.op_times;}
	std::vector<int>& get_circle_dirs()		{return circle_info.op_dirs;}
	
	/*	Printers	*/
	void print_pos_steps() {
		std::cout << "Position: " << position_steps << '\n';
	}
	void print_circle_info();
	
	/*	Motor moving and helper functions	*/
	void move();
	void move_jog_mode();
	void move_step_mode();
	void circle_move();
	void stop();
	void zero_interlock_stop();
	void zero();
	
	void reset_timer_counters();
	void calculate_accel_params();
	void setup_circle_move();
	
	/*	Getters	*/
	void get_position_steps(int &steps);
	
	/*	Set Jog Speed Steps
	 * 		Set the jog speed in terms of steps
	 */
	void set_jog_speed_steps(int steps);
	
	/*	Set Jog Speed MM
	 * 		Set the jog speed in terms of mm
	 * 		Will divide the input by 100 so one int can be passed over spi
	 */
	 void set_jog_speed_mm(int mm);

	/*	Set the move mode
	 */
	void enable_jog_mode(bool enable);
	void enable_step_mode(bool enable);
	void enable_sync_mode(bool enable);
	
	/* 	Timer Callbacks	*/
	static void IRAM_ATTR periodic_pulse_callback(void* arg);
	static void IRAM_ATTR periodic_accel_callback(void* arg);
	static void IRAM_ATTR one_shot_pulse_callback(void* arg);
	static void IRAM_ATTR one_shot_accel_callback(void* arg);
	
	/* Sync Semaphore Release Interrupt	*/
	static void IRAM_ATTR syncSem_release_isr(void* arg);
			 
private:				
	//Motor Move Variables
	int 		SPR;				//Steps per revolution
	int 		max_travel_steps;	//Max motor travel in steps
	double 		max_travel_mm;		//Max motor travel in mm
	double 		mm_per_step;		//Millimeters traveled on each step
	
	int rpm;				//Rotations per minute
	int sps;				//Steps per second
	int steps_to_move;		//Steps to move
	int pulse_period_us;	//Wait time between steps in microseconds
	int zero_steps;			//Number of steps to move away from the position that trips the zero interlock
	
	int accel_period_us;	//Period of the accel timer; how often an accel step is performed
	int accel_amount_us;	//Amount to change the pulse period one each accel step
	int accel_steps;		//Number of times the periodic accel timer will update the speed
	int accel_move_time;	//Total time the pulse timer runs during a move with accel
	int accel_move_steps;	//Number of steps motor moves during acceleration
	
	const int init_pulse_period_us = 250;	//Time between steps in microseconds (300rpm) at start of acceleration and during jog
	
	static bool motor_in_motion;		//True when the motor is in motion
	bool move_with_accel	= false;	//True if motor should accelerate to speed
	bool zeroing 			= false;	//True if motor is trying to find machine zero
	
	//Circle Move Info
	circle_info_t circle_info;
	
	//Jog Mode Variables
	int jog_once_steps = 0;			//Number of steps to move in one jog step
	
	//Position and Direction Variables
	static bool motor_direction;
	static int	position_steps;		//Position of the motor in steps from zero

	//Move Modes
	bool jog_mode = false;
	bool step_mode = false;
	bool sync_mode = false;

	//ESP Error
	esp_err_t error = ESP_OK;
		
	//Timer Arguments
	timer_args_t pulse_args;
	timer_args_t accel_args;
	
	//Semaphore for Sync Move
	static xQueueHandle syncSem;
	
	//Static Containers to Store Curve Operation Info
	static std::vector<int> curve_steps;
	static std::vector<int> curve_times;
	static std::vector<int> curve_dirs;	
	static int curve_op;
	
	//SPI Client to Toggle Ready Pin
	SpiClient *spi;
};

#endif	
