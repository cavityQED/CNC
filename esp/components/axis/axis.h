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

#define PULSE_PIN	(gpio_num_t) 15
#define DIR_PIN		(gpio_num_t) 2
#define EN_PIN		(gpio_num_t) 4
#define SYNC_PIN	(gpio_num_t) 26

//Timers
#define VECTOR_GROUP	TIMER_GROUP_0
#define PULSE_TIMER		TIMER_0
#define SECONDS_TIMER	TIMER_1

typedef enum
{
	x_axis,
	y_axis,
	z_axis,

}AXIS;

struct position_t
{
	int	x;
	int y;
	int z;

	friend bool operator==(const position_t& p1, const position_t& p2)
	{
		return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
	}

	friend std::ostream& operator<<(std::ostream& out, const position_t& p)
	{
		out << "[" << p.x << ", " << p.y << ", " << p.z << "]\n";
		return out;
	}
};

class axis {

public:

	/*	Constructor	*/
	axis() {}
	
	/*	Destructor	*/
	~axis() {}
	
	void setup_gpio();
	void setup_timers();
	void reset_timers();

	/*	Check error function; prints the input and then the error	*/
	void check_error(const char* msg);
		
	/*	Printers	*/
	void print_position() const
	{
		std::cout << "\tPosition (steps):\t\t" << m_step_position << '\n';
	}

	void print_info() const
	{
		std::cout << "\tPosition (steps):\t"	<< m_step_position << '\n';
		std::cout << "\tCurrent Divider:\t"		<< m_divider << '\n';
		std::cout << "\tCurrent Step:\t\t"		<< m_cur_pulse << '\n';
		std::cout << "\tMove Position:\t\t"		<< m_cur_pos;
	}
	
public:
	/*	Setters	*/

	void	set_axis		(AXIS a)		{m_axis = a;}
	void	set_spmm		(int spmm)		{m_spmm = spmm;}
	void	set_max_steps	(int max)		{m_max_steps = max; m_max_mm = m_max_steps / m_spmm;}
	void	set_max_mm		(double mm)		{m_max_mm = mm; m_max_steps = m_max_mm * m_spmm;}
	void	set_direction	(bool dir)		{m_direction = dir; gpio_set_level(DIR_PIN, dir);}
	void	set_accel		(int a)			{m_accel = a;}
	void	set_init_period	(int period)	{m_init_period_us = period;}
	void	set_jog_steps	(int steps)		{m_jog_steps = steps;}
	void	set_jog_speed	(int jog_us)	{m_jog_period_us = jog_us;}
	void	set_rapid_speed	(int rapid_us)	{m_rapid_period_us = rapid_us;}
	void	set_spi			(SpiClient *s)	{m_spi = s;}

public:
	/*	Getters	*/

	int 	step_position() 	{return m_step_position;}
	bool 	in_motion() 		{return m_motion;}
	bool	direction()			{return m_direction;}

public:
	/*	Set the move mode	*/

	void enable_jog_mode		(bool enable);
	void enable_line_mode		(bool enable);
	void enable_curv_mode		(bool enable);
	void enable_sync_mode		(bool enable);
		
public:
	/*	Movers	*/

	void vector_move_config(	const position_t& start,
								const position_t& end,
								int final_wait_time,
								int r = m_radius,
								bool dir = false,
								int accel = m_accel);
	void vector_move();
	void jog_move(bool dir);

public:

	static void IRAM_ATTR linear_interpolation_2D	();
	static void IRAM_ATTR circular_interpolation_2D	();
	static void IRAM_ATTR update_divider			(timer_group_t TIMER_GROUP);
	static void IRAM_ATTR vector_move_isr			(void* arg);
	
	/* Sync Semaphore Release Interrupt	*/
	static void IRAM_ATTR syncSem_release_isr(void* arg);

protected:

	static AXIS			m_axis;				//Axis - x, y, or z
	static int			m_spmm;				//Motor steps per mm 
	static int			m_max_mm;			//Maximum travel in mm
	static int			m_max_steps;		//Maximum travel in steps
	static int			m_jog_steps;		//Steps to travel during jog
	static int			m_jog_period_us;	//Jog step period in microseconds
	static int			m_rapid_period_us;	//Wait time in microseconds for rapid moves
	static int			m_init_period_us;	//Initial wait time in microseconds between step pulses	
	static SpiClient*	m_spi;				//Spi client to send messages to pi
	static esp_err_t	m_esp_err;			//ESP error
	static xQueueHandle m_syncSem;			//Sync semaphore; used to signal start of sync move

	static int			m_accel;			//Accel rate in steps/s/s
	static double		m_pulse_period_sec;	//Wait time in seconds between step pulses
	static double		m_divider_min;		//Minimum divider; 1 microsecond tick period
	static double		m_divider_max;		//Maximum divider; initial wait time (safe stopping speed) period
	static double		m_divider;			//Current divider

	static int			m_cur_pulse;		//Current pulse number
	static int			m_accel_pulse;		//Pulse to stop acceleration
	static int			m_decel_pulse;		//Pulse to start deceleration
	static int			m_final_pulse;		//Final pulse number; end point of the move
	static int			m_radius;			//Radius for a curve move
	static AXIS			m_step_axis;		//Axis that should step on timer pulse
	static double		m_cur_time;			//Current time in seconds during a move
	static double		m_accel_time;		//Time it takes to accelerate to final pulse period in seconds
	static double 		m_slope;			//Slope of a linear move
	static position_t	m_cur_pos;			//Current position during a move in steps
	static position_t	m_end_pos;			//Final position of a move in steps
	static position_t	m_jog_pos;			//Jog end position relative to current position

	static int			m_step_position;	//Motor position in steps from zero
	static bool			m_direction;		//Motor direction
	static bool			m_CW;				//True for CW curve, false for CCW
	static bool			m_motion;			//Motor in motion or not

	static bool			m_jog_mode;			/****************/
	static bool			m_line_mode;		/*	MOVE MODES	*/
	static bool			m_curv_mode;		/*				*/
	static bool			m_sync_mode;		/****************/

};

#endif	
