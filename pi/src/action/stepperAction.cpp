#include "stepperAction.h"

namespace CNC
{

StepperAction::StepperAction(CNC::DEVICE::stepperMotor* motor, CNC::codeBlock block, QWidget* parent)
	: Action(block, parent), m_stepper(motor)
{

}

StepperAction::StepperAction(CNC::StepperAction::linearConfig& config, QWidget* parent)
	: Action(config.block, parent), m_linearConfig(std::move(config))
{

}

StepperAction::StepperAction(CNC::StepperAction::vectorConfig& config, QWidget* parent)
	: Action(config.block, parent), m_vectorConfig(std::move(config))
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
			m_linearConfig.motor->esp_linear_move(	m_linearConfig.sync,
													m_linearConfig.direction,
													m_linearConfig.mm,
													m_linearConfig.seconds);
			break;
		case 2:
			//Circular Interpolation Clockwise
		case 3:
			//Circular Interpolation Counterclockwise
			m_vectorConfig.motor->vectorMove(	m_vectorConfig.xi,
												m_vectorConfig.xf,
												m_vectorConfig.yi,
												m_vectorConfig.yf,
												m_vectorConfig.block.f,
												m_vectorConfig.r,
												m_vectorConfig.dir);
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