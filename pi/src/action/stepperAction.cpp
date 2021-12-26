#include "stepperAction.h"

namespace CNC
{

StepperAction::StepperAction(const CNC::codeBlock& block, const axes_t& axes, QWidget* parent)
	: Action(parent), m_block(std::move(block)), m_axes(std::move(axes))
{

}

StepperAction::StepperAction(const stepperConfig& config, const axes_t& axes, QWidget* parent)
	: Action(parent), m_config(std::move(config)), m_axes(std::move(axes))
{
	m_block = std::move(config.block);
}

void StepperAction::execute()
{
	std::cout << "\tStepper Action Executing......\n";

	if(m_axes.x)
		m_axes.x->esp_receive();
	if(m_axes.y)
		m_axes.y->esp_receive();
	if(m_axes.z)
		m_axes.z->esp_receive();


	switch(m_block.numberCode)
	{
		case 0:
		{
			//Rapid
			m_block.f = -1;
		}

		case 1:
		{
			//Linear Interpolation

			if(m_axes.x)
				m_config.xf = (m_block.abs_x)? m_block.x - m_axes.x->mmPosition() : m_block.u;
			if(m_axes.y)
				m_config.yf = (m_block.abs_y)? m_block.y - m_axes.y->mmPosition() : m_block.v;
			if(m_axes.z)
				m_config.zf = (m_block.abs_z)? m_block.z - m_axes.z->mmPosition() : m_block.w;

			if(m_config.xf)
			{
				m_axes.x->esp_vector_move(m_config.xf, m_config.yf, m_config.zf, m_block.f);			
				m_axes.x->spiWaitForReady();
			}
			if(m_config.yf)
			{
				m_axes.y->esp_vector_move(m_config.xf, m_config.yf, m_config.zf, m_block.f);
				m_axes.y->spiWaitForReady();
			}
			if(m_config.zf)
			{
				m_axes.z->esp_vector_move(m_config.xf, m_config.yf, m_config.zf, m_block.f);
				m_axes.z->spiWaitForReady();
			}

			//Trigger sync pin
			gpioWrite(m_syncPin, 1);
			gpioWrite(m_syncPin, 0);
								
			break;
		}

		case 2:
			//Circular Interpolation Clockwise
		case 3:
		{
			//Circular Interpolation Counterclockwise

			m_config.xi = -m_block.i;
			m_config.yi = -m_block.j;

			m_config.xf = (m_block.abs_x)? m_block.x - m_axes.x->mmPosition() : m_block.u;
			m_config.yf = (m_block.abs_y)? m_block.y - m_axes.y->mmPosition() : m_block.v;

	 		m_config.xf -= m_block.i;
			m_config.yf -= m_block.j;

			m_config.r	= std::sqrt(m_block.i*m_block.i + m_block.j*m_block.j);

			m_config.dir	= !(m_block.numberCode % 2);
			m_config.f		= m_block.f;
		
			m_axes.x->esp_circle_move(m_config.xi, m_config.yi, m_config.xf, m_config.yf, m_config.r, m_config.f, m_config.dir);
			m_axes.x->spiWaitForReady();
			
			m_axes.y->esp_circle_move(m_config.xi, m_config.yi, m_config.xf, m_config.yf, m_config.r, m_config.f, m_config.dir);
			m_axes.y->spiWaitForReady();

			//Trigger sync pin
			gpioWrite(m_syncPin, 1);
			gpioWrite(m_syncPin, 0);

			break;
		}

		case 4:
			//Dwell
			break;
		default:
			//Unsupported G code
			break;
	}
	std::cout << "\tStepper Action Executed\n";
}

}//CNC namespace