#include "stepperGroup.h"

namespace CNC
{

namespace DEVICE
{

StepperGroup::StepperGroup(QWidget* parent) : QWidget(parent)
{
	gpioSetMode(SYNC_PIN, PI_OUTPUT);
}

bool StepperGroup::addStepper(CNC::AXIS axis, CNC::DEVICE::stepperMotor* stepper)
{
	auto pair = m_steppers.insert(std::make_pair(axis, stepper));
	return pair.second;
}

void StepperGroup::update()
{
	m_motion = false;

	for(auto s : m_steppers)
	{
		s.second->esp_receive();
		m_motion = m_motion || s.second->isMoving();
	}
}

void StepperGroup::pause()
{
	for(auto s : m_steppers)
		s.second->esp_timer_pause(true);
}

void StepperGroup::resume()
{
	for(auto s : m_steppers)
		s.second->esp_timer_pause(false);
}

void StepperGroup::stop()
{
	for(auto s : m_steppers)
		s.second->esp_stop();
}

void StepperGroup::homeAll()
{
	for(auto s : m_steppers)
		s.second->esp_find_zero();
}

void StepperGroup::executeBlock(const CNC::codeBlock* b)
{
	update();
	m_startPosition = {0,0,0};
	m_endPosition = {0,0,0};

	auto args = b->args();

	if(args.count('F'))
		m_feedRate = args.at('F');

	double feedRate = m_feedRate;

	switch(b->m_numberCode)
	{
		case 0:
			feedRate = -1;

		case 1:		//Linear Interpolation
		{
			for(auto arg : args)
			{
				switch(arg.first)
				{
					case 'U':	m_endPosition.x = arg.second;											break;
					case 'V':	m_endPosition.y = arg.second;											break;
					case 'X':	m_endPosition.x = arg.second - m_steppers[CNC::AXIS::X]->mmPosition();	break;
					case 'Y':	m_endPosition.y = arg.second - m_steppers[CNC::AXIS::Y]->mmPosition();	break;

				}
			}
			
			for(auto s : m_steppers)
			{
				s.second->esp_vector_move(	m_endPosition.x,
											m_endPosition.y,
											m_endPosition.z,
											feedRate);

				s.second->spiWaitForReady();
			}

			gpioWrite(SYNC_PIN, 1);
			gpioWrite(SYNC_PIN, 0);

			//update();

			break;
		}

		case 2:		//Circular Interpolation CW
		case 3:		//Circular Interpolation CCW
		{
			for(auto arg : args)
			{
				switch(arg.first)
				{
					case 'I':	m_startPosition.x = -arg.second;										break;
					case 'J':	m_startPosition.y = -arg.second;										break;
					case 'X':	m_endPosition.x = arg.second - m_steppers[CNC::AXIS::X]->mmPosition();	break;
					case 'Y':	m_endPosition.y = arg.second - m_steppers[CNC::AXIS::Y]->mmPosition();	break;
					case 'U':	m_endPosition.x = arg.second;											break;
					case 'V':	m_endPosition.y = arg.second;											break;
				}
			}

			m_endPosition.x += m_startPosition.x;
			m_endPosition.y += m_startPosition.y;

			for(auto s : m_steppers)
			{
				s.second->esp_circle_move(	m_startPosition.x,
											m_startPosition.y,
											m_endPosition.x,
											m_endPosition.y,
											m_startPosition.length(),
											feedRate,
											!(b->m_numberCode % 2));
				s.second->spiWaitForReady();
			}

			gpioWrite(SYNC_PIN, 1);
			gpioWrite(SYNC_PIN, 0);

			//update();

			break;
		}

		case 28:	//Return to Machine Zero
		{
			if(args.empty())
				homeAll();

			else
			{
				for(auto arg : args)
				{
					switch(arg.first)
					{
						case 'X': case 'U':	m_steppers[CNC::AXIS::X]->esp_find_zero();	break;
						case 'Y': case 'V': m_steppers[CNC::AXIS::Y]->esp_find_zero();	break;
					}
				}
			}

			break;
		}

		default:
			break;
	}
}

}//DEVICE namespace
}//CNC namespace