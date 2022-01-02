#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include "spiDevice.h"
#include "common.h"

#include <QTimer>
#include <QTimerEvent>

namespace CNC
{
namespace DEVICE
{

namespace ESP
{
//Function to send the esp32
enum esp32FUNCTION
{
	SET_AXIS,
	SET_STEPS_PER_MM,
	SET_MAX_STEPS,

	SET_DIRECTION,
	SET_ACCELERATION,
	SET_INITIAL_PERIOD,

	SET_JOG_STEPS,
	SET_JOG_SPEED,
	SET_RAPID_SPEED,

	ENA_JOG_MODE,		
	ENA_LINE_MODE,
	ENA_CURV_MODE,
	ENA_SYNC_MODE,

	VECTOR_MOVE,
	CIRCLE_MOVE,
	SCALAR_MOVE,
	JOG_MOVE,
	STOP,
	PAUSE_TIMERS,
	RECEIVE,

	FIND_ZERO,
};

enum AXIS
{
	x_axis,
	y_axis,
	z_axis,
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

		ESP::AXIS axis;	//x, y, or z axis
	};

	stepperMotor(params_t &params, QWidget* parent = nullptr);
	~stepperMotor() {}

	/*	Configure Stepper
	*		Set Stepper Motor Parameters
	*			-	steps/mm
	*			-	max travel
	*			-	x axis
	*/
	void configureStepper(params_t &p);

	void setStepOffset(int steps)	{m_stepOffset = steps;}

public:

	virtual bool isReady() {esp_receive(); return m_inMotion;}

public:

	/****************************
	*	ESP FUNCTION COMMANDS	*
	****************************/
	/*
	*	Functions to send individual commands over spi
	*		to the esp controller responsible for driving the motor
	*/

	void esp_set_axis			(ESP::AXIS a);
	void esp_set_steps_per_mm	(int steps);
	void esp_set_max_steps		(int max_steps);

	void esp_set_direction		(bool dir);
	void esp_set_acceleration	(int accel);
	void esp_set_initial_period	(int period_us);

	void esp_set_jog_steps		(int steps);
	void esp_set_jog_speed		(int jog_us);
	void esp_set_rapid_speed	(int rapid_us);

	void esp_enable_jog_mode	(bool enable);
	void esp_enable_line_mode	(bool enable);
	void esp_enable_curv_mode	(bool enable);
	void esp_enable_sync_mode	(bool enable);

	void esp_vector_move		(double dx, double dy, double dz, double f);
	void esp_circle_move		(double xi, double yi, double xf, double yf, double r, double f, bool cw);
	void esp_jog_move			(bool dir);
	void esp_receive			();
	void esp_stop				();
	void esp_timer_pause		(bool pause);

	void esp_find_zero			();

public slots:

	void setAxis(CNC::AXIS a)		{	m_axis = a;														}
	void setMode(CNC::MODE m)		{	m_mode = m;														}
	void setHome()					{	esp_receive(); m_stepOffset = m_stepPosition; esp_receive();	}
	void jogMove(bool dir)			{	esp_jog_move(dir);												}
	void jogEnable(bool ena)		{	esp_enable_jog_mode(ena);										}
	void setJogDistance(double mm)	{	esp_set_jog_steps(mm*m_params.spmm);							}

public:

	virtual void timerEvent(QTimerEvent* e) override;

signals:

	void positionChange(double mm);

public:

	//Getters
	int			stepPosition()		{return m_stepPosition - m_stepOffset;}
	int			absStepPosition()	{return m_stepPosition;}
	bool		isMoving()			{return m_inMotion;}
	bool		x_axis()			{return m_params.axis;}
	double		mmPosition()		{return stepPosition()/(double)m_params.spmm;}
	params_t&	params()			{return m_params;}

protected:

	params_t	m_params;
	int			m_stepPosition;		//Absolute motor position in steps from zero
	int			m_stepOffset = 0;	//Offset for work coordinates
	double		m_mmPosition;		//Absolute motor position in mm from zero
	bool		m_inMotion;			//True if the motor is in motion
	bool		m_sync;				//True if action is part of a sync action
	bool		m_homed = false;	//True if the motor has been homed out

	int			m_jogSteps;			//Number of steps to move on a jog
	int			m_jogTime;			//Jog step wait time in us

	//Modes
	bool		m_jogMode = false;
	bool		m_lineMode = false;
	bool		m_curvMode = false;
	bool		m_syncMode = false;

	CNC::MODE	m_mode 		= CNC::MODE::HOME;
	CNC::AXIS	m_axis;


	static const int m_timerPeriod = 10;
};

}//DEVICE namespace
}//CNC namespace

#endif