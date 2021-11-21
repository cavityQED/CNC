#include "laserAction.h"

namespace CNC
{

LaserAction::LaserAction(	CNC::DEVICE::Laser* laser,
							CNC::codeBlock block,
							QWidget* parent)
	: Action(block, parent), m_laser(laser)
{

}

void LaserAction::execute()
{
	if(m_laser == nullptr)
		return;

	std::cout << "\nLaser Action Executing......\n";

	switch(m_block.numberCode)
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
			m_laser->setPower(m_block.p, true);
			break;
		default:
			break;
	}

	std::cout << "\nLaser Action Executed\n";

}

}//CNC namespace