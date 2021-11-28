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

	StepperAction(CNC::DEVICE::stepperMotor* motor, CNC::codeBlock block, QWidget* parent = nullptr);
	StepperAction(CNC::StepperAction::stepperConfig& config, QWidget* parent = nullptr);
	~StepperAction() {}

	virtual void enable_sync_mode(bool enable) override {m_stepper->esp_enable_sync_mode(enable);}
	virtual void wait_for_ready() override				{m_stepper->spiWaitForReady();}

public:

public slots:
	
	virtual void execute() override;

protected:

	CNC::DEVICE::stepperMotor*	m_stepper;
	stepperConfig				m_config;

	bool	m_dir;				//Direction - true for positive move, false for negative
	int		m_steps;			//Number of steps to move
	int		m_step_time_us;		//Time per step in microseconds
	double	m_move_distance_mm;	//Total distance moved in mm
	double	m_move_time_sec;	//Total move time in seconds
};

}//CNC namespace

#endif