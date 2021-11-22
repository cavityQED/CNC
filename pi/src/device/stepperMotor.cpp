#include "stepperMotor.h"

namespace CNC
{

namespace DEVICE
{

stepperMotor::stepperMotor(params_t &params, QWidget* parent) : spiDevice(parent), m_params(std::move(params))
{
	//Setup the device pin
	gpioSetMode(m_params.device_pin, PI_OUTPUT);
	gpioWrite(m_params.device_pin, 0);

	esp_motor_config(m_params);
}

void stepperMotor::linearMove(bool sync, bool dir, double mm, double time)
{
	esp_enable_line_mode(true);
	esp_enable_sync_mode(sync);
	esp_set_direction(dir);
	esp_set_steps_to_move(mm * m_params.spmm);
	esp_set_step_time(1000000 * time / (mm * (double)m_params.spmm));
	esp_move();
}

void stepperMotor::esp_motor_config(params_t &p)
{
	sendBuffer[0] = ESP::SET_X_AXIS;
	sendBuffer[1] = p.x_axis;
	spiSend(m_params.device_pin);

	sendBuffer[0] = ESP::SET_STEPS_PER_MM;
	sendBuffer[1] = p.spmm;
	spiSend(m_params.device_pin);

	sendBuffer[0] = ESP::SET_MAX_STEPS;
	sendBuffer[1] = p.max_mm;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_move()
{
	sendBuffer[0] = ESP::MOVE;
	spiSend(m_params.device_pin);
	m_inMotion = true;
}

void stepperMotor::esp_linear_move(bool sync, bool dir, double mm, double time)
{
	sendBuffer[0] = ESP::LINEAR_MOVE;
	sendBuffer[1] = (int)sync;
	sendBuffer[2] = (int)dir;
	sendBuffer[3] = mm * m_params.spmm;
	sendBuffer[4] = 1000000.0 * time / mm / m_params.spmm;
	spiSend(m_params.device_pin);
}


void stepperMotor::move_to(double mm_pos, bool sync)
{
	//If the motor is in motion already, do nothing
	if(m_inMotion)
	{
		std::cout << "Motor in motion!\n";
		return;
	}

	double diff	= m_mmPosition - mm_pos;

	//Set the direction
	esp_set_direction(!std::signbit(diff));

	//Set the steps to move
	esp_set_steps_to_move(std::fabs(diff) + m_params.spmm);

	//Set the sync mode
	esp_enable_sync_mode(sync);

	//Finally, tell the device to move
	esp_move();
}

void stepperMotor::move_to(int step_pos, bool sync)
{

}

void stepperMotor::move_steps(int steps, bool dir, bool sync)
{

}

void stepperMotor::esp_set_steps_to_move(int steps)
{
	std::cout << "\n";
	sendBuffer[0] = ESP::SET_STEPS_TO_MOVE;
	sendBuffer[1] = steps;
	spiSend(m_params.device_pin);

}

void stepperMotor::esp_set_jog_steps(int steps)
{
	sendBuffer[0] = ESP::SET_JOG_STEPS;
	sendBuffer[1] = steps;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_direction(bool dir)
{
	sendBuffer[0] = ESP::SET_DIRECTION;
	sendBuffer[1] = (int)dir;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_step_time(int time_us)
{
	sendBuffer[0] = ESP::SET_STEP_TIME;
	sendBuffer[1] = time_us;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_jog_mode(bool enable)
{
	sendBuffer[0] = ESP::ENA_JOG_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_line_mode(bool enable)
{
	sendBuffer[0] = ESP::ENA_LINE_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_curv_mode(bool enable)
{
	sendBuffer[0] = ESP::ENA_CURV_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_sync_mode(bool enable)
{
	sendBuffer[0] = ESP::ENA_SYNC_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_get_motion_info()
{
	sendBuffer[0] = ESP::RECEIVE;
	spiSend(m_params.device_pin);

	m_stepPosition = recvBuffer[1];
	m_inMotion = recvBuffer[2];

	m_mmPosition = (double)m_stepPosition/(double)m_params.spmm;
	emit positionChange(m_mmPosition);
}
	

}//DEVICE namespace
}//CNC namespace