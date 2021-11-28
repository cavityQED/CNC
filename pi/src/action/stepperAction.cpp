#include "stepperAction.h"

namespace CNC
{

StepperAction::StepperAction(CNC::DEVICE::stepperMotor* motor, CNC::codeBlock block, QWidget* parent)
	: Action(block, parent), m_stepper(motor)
{

}

StepperAction::StepperAction(CNC::StepperAction::stepperConfig& config, QWidget* parent)
	: Action(config.block, parent), m_config(std::move(config))
{

}


void StepperAction::execute()
{
	std::cout << "\nStepper Action Executing......\n";


	switch(m_block.numberCode)
	{
		case 0:
			//Rapid positioning
			break;
		case 1:
			//Linear Interpolation
			m_config.motor->esp_enable_line_mode(true);

			m_config.motor->vectorMove(	m_config.xi,
										m_config.yi,
										m_config.zi,
										m_config.xf,
										m_config.yf,
										m_config.zf,
										m_config.f);
			break;
		case 2:
			//Circular Interpolation Clockwise
		case 3:
			//Circular Interpolation Counterclockwise
			m_config.motor->vectorMove(	m_config.xi,
										m_config.yi,
										m_config.zi,
										m_config.xf,
										m_config.yf,
										m_config.zf,
										m_config.f,
										m_config.r,
										m_config.dir);
			break;
		case 4:
			//Dwell
			break;
		default:
			//Unsupported G code
			break;
	}
	std::cout << "\nStepper Action Executed\n";
}

}//CNC namespace