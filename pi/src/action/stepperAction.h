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

	struct params_t
	{
		int		type;
		double	seconds;
		double	mm;
		bool	direction;
		bool	sync;

		CNC::DEVICE::stepperMotor*	motor;
		CNC::codeBlock				block;
	};

	StepperAction(CNC::DEVICE::stepperMotor* motor, CNC::codeBlock block, QWidget* parent = nullptr);
	StepperAction(CNC::StepperAction::params_t& p, QWidget* parent = nullptr);
	~StepperAction() {}

	virtual void enable_sync_mode(bool enable) override {m_stepper->esp_enable_sync_mode(enable);}
	virtual void wait_for_ready() override				{m_stepper->spiWaitForReady();}

	void setParams(const params_t& p)	{m_params = p;}

public:

public slots:
	
	virtual void execute() override;

protected:

	CNC::DEVICE::stepperMotor*	m_stepper;
	params_t					m_params;

	bool	m_dir;				//Direction - true for positive move, false for negative
	int		m_steps;			//Number of steps to move
	int		m_step_time_us;		//Time per step in microseconds
	double	m_move_distance_mm;	//Total distance moved in mm
	double	m_move_time_sec;	//Total move time in seconds
};

}//CNC namespace

#endif