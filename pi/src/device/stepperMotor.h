#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include "spiDevice.h"

namespace CNC
{
namespace DEVICE
{

namespace ESP
{
//Function to send the esp32
enum esp32FUNCTION
{
	SET_FEED_RATE, 			
	SET_DIRECTION,
	SET_STEP_TIME,
	SET_STEPS_TO_MOVE,
	SET_JOG_STEPS,
	SET_BACKLASH,
	SET_X_AXIS,
	SET_STEPS_PER_MM,
	SET_MAX_STEPS,
	SETUP_CURVE,
	ENA_JOG_MODE,		
	ENA_LINE_MODE,
	ENA_CURV_MODE,
	ENA_SYNC_MODE,
	ENA_JOG_CONTINUOUS,
	ENA_TRAVEL_LIMITS,
	FIND_ZERO,
	MOVE,
	LINEAR_MOVE,
	STOP,
	RECEIVE,
	TEST_FUNCTION
};
}//ESP namespace

class stepperMotor : public spiDevice
{
	Q_OBJECT

public:

	struct params_t
	{
		int device_pin;	//Pin used to signal device for SPI transaction
		int spmm;		//Motor steps in 1mm of linear travel
		int max_mm;		//Maximum travel in mm

		bool x_axis;	//Will change to an axis enum later
	};

	stepperMotor(params_t &params, QWidget* parent = nullptr);
	~stepperMotor() {}

	void linearMove(bool sync, bool dir, double mm, double time);

	//ESP32 motor config
	void esp_motor_config(params_t &p);

	//ESP32 move functions
	void esp_move();
	void esp_linear_move(bool sync, bool dir, double mm, double time);
	void move_to(double	mm_pos, bool sync = true);
	void move_to(int 	step_pos, bool sync = true);
	void move_steps(int steps, bool dir, bool sync = false);

	//ESP32 move config
	void esp_set_steps_to_move(int steps);
	void esp_set_jog_steps(int steps);
	void esp_set_direction(bool dir);
	void esp_set_step_time(int time_us);	//Set the step timer in microseconds

	//ESP32 mode config
	void esp_enable_jog_mode(bool enable);
	void esp_enable_line_mode(bool enable);
	void esp_enable_curv_mode(bool enable);
	void esp_enable_sync_mode(bool enable);

	//ESP32 recieve motion info
	void esp_get_motion_info();

public:

	//Getters
	bool		isMoving()		{return m_inMotion;}
	bool		stepPosition()	{return m_stepPosition;}
	bool		x_axis()		{return m_params.x_axis;}
	params_t&	params()		{return m_params;}

protected:

	params_t	m_params;
	int			m_stepPosition;		//Absolute motor position in steps from zero
	double		m_mmPosition;		//Absolute motor position in mm from zero
	bool		m_inMotion;			//True if the motor is in motion
	bool		m_sync;				//True if action is part of a sync action
};

}//DEVICE namespace
}//CNC namespace

#endif