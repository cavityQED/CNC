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
	esp_set_axis			(p.axis);
	esp_set_steps_per_mm	(p.spmm);
	esp_set_max_steps		(p.max_mm * p.spmm);
}

void stepperMotor::vectorMove(	double xi,
								double yi,
								double zi,
								double xf,
								double yf,
								double zf,
								double f,
								double r,
								bool dir)
{
	sendBuffer[0] = ESP::VECTOR_MOVE;
	sendBuffer[1] = xi * m_params.spmm;
	sendBuffer[2] = yi * m_params.spmm;
	sendBuffer[3] = zi * m_params.spmm;
	sendBuffer[4] = xf * m_params.spmm;
	sendBuffer[5] = yf * m_params.spmm;
	sendBuffer[6] = zf * m_params.spmm;
	sendBuffer[7] = 1000000 / (int)(f * m_params.spmm);
	sendBuffer[8] = (int)(r * m_params.spmm);
	sendBuffer[9] = (int)dir;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_axis(ESP::AXIS a)
{
	sendBuffer[0] = ESP::SET_AXIS;
	sendBuffer[1] = (int)a;
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

void stepperMotor::esp_set_jog_steps(int jog_steps)
{
	sendBuffer[0] = ESP::SET_JOG_STEPS;
	sendBuffer[1] = jog_steps;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_jog_speed(int jog_us)
{
	sendBuffer[0] = ESP::SET_JOG_SPEED;
	sendBuffer[1] = jog_us;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_set_rapid_speed(int rapid_us)
{
	sendBuffer[0] = ESP::SET_RAPID_SPEED;
	sendBuffer[1] = rapid_us;
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

void stepperMotor::esp_vector_move(double dx, double dy, double dz, double f)
{
	sendBuffer[0] = ESP::VECTOR_MOVE;
	sendBuffer[1] = dx * m_params.spmm;
	sendBuffer[2] = dy * m_params.spmm;
	sendBuffer[3] = dz * m_params.spmm;
	sendBuffer[4] = 1000000 / (int)(f * m_params.spmm);
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_circle_move(double xi, double yi, double xf, double yf, double f, double r, bool cw)
{
	sendBuffer[0] = ESP::CIRCLE_MOVE;
	sendBuffer[1] = std::round(xi * (double)m_params.spmm);
	sendBuffer[2] = std::round(yi * (double)m_params.spmm);
	sendBuffer[3] = std::round(xf * (double)m_params.spmm);
	sendBuffer[4] = std::round(yf * (double)m_params.spmm);
	sendBuffer[5] = 1000000 / (int)(f * m_params.spmm);
	sendBuffer[6] = std::round(r * (double)m_params.spmm);
	sendBuffer[7] = (int)cw;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_jog_move(bool dir)
{
	sendBuffer[0] = ESP::JOG_MOVE;
	sendBuffer[1] = (int)dir;
	spiSend(m_params.device_pin);
	startTimer(m_timerPeriod);
}

void stepperMotor::esp_receive()
{
	sendBuffer[0] = ESP::RECEIVE;
	spiSend(m_params.device_pin);

	m_stepPosition = recvBuffer[1];
	m_inMotion = (bool)recvBuffer[2];

	emit positionChange(mmPosition());
}

void stepperMotor::esp_stop()
{
	sendBuffer[0] = ESP::STOP;
	spiSend(m_params.device_pin);
	startTimer(m_timerPeriod);
}

void stepperMotor::esp_timer_pause(bool pause)
{
	sendBuffer[0] = ESP::PAUSE_TIMERS;
	sendBuffer[1] = (int)pause;
	spiSend(m_params.device_pin);
}

void stepperMotor::esp_find_zero()
{
	esp_receive();

	if(m_stepPosition == 0 && m_homed)
		return;
	
	else if(!m_homed)
	{
		sendBuffer[0] = ESP::FIND_ZERO;
		spiSend(m_params.device_pin);
		startTimer(m_timerPeriod);

		//Shouldn't really put this here in case it doesn't actually find home
		m_homed = true;
	}

	else if(m_stepPosition && m_homed)
	{
		sendBuffer[0] = ESP::SCALAR_MOVE;
		sendBuffer[1] = std::abs(m_stepPosition);
		sendBuffer[2] = std::signbit(m_stepPosition);
		spiSend(m_params.device_pin);
		startTimer(m_timerPeriod);
	}
}

void stepperMotor::timerEvent(QTimerEvent* e)
{
	esp_receive();
	if(!m_inMotion)
		killTimer(e->timerId());
}

}//DEVICE namespace
}//CNC namespace