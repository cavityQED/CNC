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

	configureStepper(m_params);
}

void stepperMotor::configureStepper(params_t &p)
{
	esp_set_x_axis			(p.x_axis);
	esp_set_steps_per_mm	(p.spmm);
	esp_set_max_steps		(p.max_mm * p.spmm);
}

void stepperMotor::jogMove(bool dir)
{
	if(!m_jogMode)
		return;

	esp_set_direction(dir);
	esp_move();
	startTimer(m_timerPeriod);
}

void stepperMotor::linearMove(bool sync, bool dir, double mm, double time)
{
	esp_linear_move(sync, dir, mm, time);
}

void stepperMotor::vectorMove(double xi, double xf, double yi, double yf, double feed, double r, bool dir)
{
	std::cout << "\tVector Move: " << "[" << xi << ", " << xf << ", " << yi << ", "  << yf << ", " << feed << ", " << r << ", " << dir << "]\n"; 
	esp_enable_sync_mode(true);
	esp_enable_curv_mode(true);
	esp_setup_curve(xi, xf, yi, yf, feed, r, dir);
	spiWaitForReady();
	std::cout << "\tSPI Wait Released - Moving Motor\n";
	esp_move();
}

void stepperMotor::esp_move()
{
	sendBuffer[0] = ESP::MOVE;
	spiSend(m_params.device_pin);
	m_inMotion = true;
}

void stepperMotor::esp_stop()
{
	sendBuffer[0] = ESP::STOP;
	spiSend(m_params.device_pin);
	startTimer(m_timerPeriod);
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

void stepperMotor::esp_set_steps_per_mm(int steps)
{
	sendBuffer[0] = ESP::SET_STEPS_PER_MM;
	sendBuffer[1] = steps;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_max_steps(int max_steps)
{
	sendBuffer[0] = ESP::SET_MAX_STEPS;
	sendBuffer[1] = max_steps;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_x_axis(bool x_axis)
{
	sendBuffer[0] = ESP::SET_X_AXIS;
	sendBuffer[1] = (int)x_axis;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_setup_curve(double xi, double xf, double yi, double yf, double feed, double r, bool dir)
{
	sendBuffer[0] = ESP::SETUP_CURVE;
	sendBuffer[1] = (int)dir;
	sendBuffer[2] = (int)(r * m_params.spmm);
	sendBuffer[3] = (int)(xi * m_params.spmm);
	sendBuffer[4] = (int)(yi * m_params.spmm);
	sendBuffer[5] = (int)(xf * m_params.spmm);
	sendBuffer[6] = (int)(yf * m_params.spmm);
	sendBuffer[7] = feed;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_jog_mode(bool enable)
{
	m_jogMode = enable;
	sendBuffer[0] = ESP::ENA_JOG_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_line_mode(bool enable)
{
	m_lineMode = enable;
	sendBuffer[0] = ESP::ENA_LINE_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_curv_mode(bool enable)
{
	m_curvMode = enable;
	sendBuffer[0] = ESP::ENA_CURV_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_enable_sync_mode(bool enable)
{
	m_syncMode = enable;
	sendBuffer[0] = ESP::ENA_SYNC_MODE;
	sendBuffer[1] = (int)enable;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_receive()
{
	sendBuffer[0] = ESP::RECEIVE;
	spiSend(m_params.device_pin);

	m_stepPosition = recvBuffer[1];
	m_inMotion = recvBuffer[2];

	m_mmPosition = (double)m_stepPosition/(double)m_params.spmm;
	emit positionChange(m_mmPosition);
}

void stepperMotor::timerEvent(QTimerEvent* e)
{
	esp_receive();
	if(!m_inMotion)
		killTimer(e->timerId());
}

}//DEVICE namespace
}//CNC namespace