#include "laserAction.h"

namespace CNC
{

LaserAction::LaserAction(std::shared_ptr<CNC::codeBlock> block, QWidget* parent) : Action(block, parent)
{

}

void LaserAction::execute()
{
	if(m_laser == nullptr)
		return;

	switch(m_block->numberCode)
	{
		case 0:
			//Laser Off
			m_laser->off();
			break;
		case 1:
			//Laser On at currently set laser power
			m_laser->on();
			break;
		case 2:
			//Laser On with power P
			m_laser->setPower(m_block->p, true);
			break;
		default:
			break;
	}
}

}//CNC namespace