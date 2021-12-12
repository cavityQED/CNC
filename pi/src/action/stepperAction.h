#ifndef STEPPERACTION_H
#define STEPPERACTION_H

#include "action.h"
#include "device/stepperMotor.h"

namespace CNC
{

class StepperAction : public Action
{
	Q_OBJECT

public:

	struct axes_t
	{
		CNC::DEVICE::stepperMotor* x = nullptr;
		CNC::DEVICE::stepperMotor* y = nullptr;
		CNC::DEVICE::stepperMotor* z = nullptr;
	};

	struct stepperConfig
	{
		double xi	= 0;
		double yi	= 0;
		double zi	= 0;
		
		double xf	= 0;
		double yf	= 0;
		double zf	= 0;

		double r	= 0;
		double f	= 0;
		
		bool dir	= 0;

		CNC::DEVICE::stepperMotor*	motor;
		CNC::codeBlock				block;
	};

	StepperAction(const CNC::codeBlock& block, const axes_t& axes, QWidget* parent = nullptr);
	StepperAction(const stepperConfig& config, const axes_t& axes, QWidget* parent = nullptr);
	~StepperAction() {}

public:

	void setAxes(const axes_t& a)	{m_axes = a;}

public slots:
	
	virtual void execute() override;

protected:

	axes_t			m_axes;
	CNC::codeBlock	m_block;
	stepperConfig	m_config;

	int				m_syncPin = 18;

	bool	m_dir;				//Direction - true for positive move, false for negative
	int		m_steps;			//Number of steps to move
	int		m_step_time_us;		//Time per step in microseconds
	double	m_move_distance_mm;	//Total distance moved in mm
	double	m_move_time_sec;	//Total move time in seconds
};

}//CNC namespace

#endif