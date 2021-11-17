#include "stepperAction.h"

namespace CNC
{

StepperAction::StepperAction(std::shared_ptr<CNC::codeBlock> block, QWidget* parent) : Action(block, parent)
{

}


void StepperAction::execute()
{
	switch(m_block->numberCode)
	{
		case 0:
			//Rapid positioning
			break;
		case 1:
			//Linear Interpolation
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
}

}//CNC namespace