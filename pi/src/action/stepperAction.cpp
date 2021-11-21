#include "stepperAction.h"

namespace CNC
{

StepperAction::StepperAction(CNC::DEVICE::stepperMotor* motor, CNC::codeBlock block, QWidget* parent)
	: Action(block, parent), m_stepper(motor)
{

}

StepperAction::StepperAction(CNC::StepperAction::params_t& p, QWidget* parent)
	: Action(p.block, parent), m_params(std::move(p))
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
			m_params.motor->linearMove(m_params.sync, m_params.direction, m_params.mm, m_params.seconds);
			break;
		case 2:
			//Circular Interpolation Clockwise
			break;
		case 3:
			//Circular Interpolation Counterclockwise
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